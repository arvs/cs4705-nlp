# coding: utf-8
#!/usr/bin/python
__author__="Arvind Srinivasan <vs2371@columbia.edu>"
import re
import csv
import math
import optparse
import shutil
from collections import defaultdict
from count_freqs import Hmm

"""
random assertions that are verified as correct to avoid dumb mistakes
"""
def test_minimal():
	t = Tagger()
	assert t.compute_emission('University','I-ORG'), 0.001299870013
	t_rare = Tagger('ner_train-rare.dat')
	assert t_rare.compute_emission('University','I-ORG'), 0.00130039011704


"""
takes argmax of tag probabilities using the model specfied in Question 4 and writes results to q4_prediction_file and q4_prediction_file-rare.
"""
def write_tag_probabilities(rare=True,num_lines=60000):
	t = Tagger("ner_train-rare.dat") if rare else Tagger("ner_train.dat")
	outfile = "q4_prediction_file-rare" if rare else "q4_prediction_file"
	with open(outfile, "w") as f1, open("ner_dev.dat") as f:
		for line in f:
			if line != '\n':
				p = t.tag_probabilities(line.strip())
				max_prob = max(p, key=p.get)
				# can't take log of 0, so defaults
				f1.write("%s %s %s\n" % (line.strip(), max_prob, ln(p[max_prob])))
			else:
				f1.write(line)

def interactive_log_prob():
	t = Tagger()
	while True:
		try:
			trigram = raw_input("\n Enter the trigram - yi-2, yi-1, yi separated by spaces: ").split(' ')
			trigram.reverse()
			print "Log Probability: %s" % ln(t.compute_trigram(*trigram))
		except EOFError:
			break

def test_viterbi(rare=False):
	t = Tagger("ner_train-rare.dat") if rare else Tagger("ner_train.dat")
	outfile = "viterbi_prediction_file-rare" if rare else "viterbi_prediction_file"
	t.viterbi('ner_dev.dat',outfile,num_lines=60000)

"""
The core tagger, implements the functionality for q4-q6. Imports functionality from count_freqs directly 
to avoid lots of file i/o
"""
class Tagger(object):
	def __init__(self, infile="ner_train.dat"):
		self.counter = Hmm(3)
		with open(infile) as f:
			self.counter.train(f)
		self.unigrams = {k[0]:v for k,v in self.counter.ngram_counts[0].iteritems()}
		self.bigrams = self.counter.ngram_counts[1]
		self.trigrams = self.counter.ngram_counts[2]
	"""
	conditional probability that the word maps to tag given the number of times the tag occurs
	"""
	def compute_emission(self, word, tag):
		em = self.counter.emission_counts
		if tag == '*':
			return 0
		if (word,tag) in em:
			return em[(word,tag)]/float(self.unigrams[tag])
		else:
			# print "%s : %s" % (('_RARE_',tag),em[('_RARE_',tag)])
			return em[('_RARE_',tag)]/float(self.unigrams[tag])

	def compute_trigram(self,yi,y1,y2):
		return float(self.trigrams.get((y2,y1,yi),0))/self.bigrams.get((y2,y1),1)

	"""
	basic file replacement, writes to a new file called rare-{infile} where infile is provided. Can pass a threshold of how many common_words
	is considered "rare"
	"""
	def replace_rare(self,infile,threshold=5):
		wordcounts = defaultdict(int)
		for tup in self.counter.emission_counts.iteritems():
			wordcounts[tup[0][0]] += tup[1] #aggregates counts of words total, with any tag
		common_words = [k for k,v in wordcounts.iteritems() if v >= threshold]
		replaced = 0
		f = open(infile)
		f2 = open(infile.replace('.dat','-rare.dat'), 'w')
		for line in f:
			if len(line.split(' ')) == 2:
				if line.split(' ')[0] not in common_words: #closed set, there are more rare than not rare, we know it's one or the other
					f2.write(line.replace(line.split(' ')[0], '_RARE_', 1))
					replaced +=1
				else:
					f2.write(line)
			else:
				f2.write(line) # probably not necessary to write empty lines, but useful to check that it didn't lose data
		f.close()
		f2.close()
		print replaced
	"""
	returns a dictionary of relative probabilities for emission counts
	"""
	def tag_probabilities(self,word):
		counts = {tag:self.compute_emission(word,tag) for tag in self.unigrams}
		prob = lambda v: v/sum(counts.values()) if sum(counts.values()) != 0 else 0
		return {k:prob(v) for k,v in counts.iteritems()}

	def viterbi(self,infile,outfile,num_lines):
		with open(infile) as f, open(outfile,"w") as f2:
			possible_tags = self.unigrams.keys()
			possible_tags.append('*')
			mat = defaultdict(lambda: 1)
			mat [(0,'*','*')] = 1
			idx = 0
			for line in f:
				if line == '\n':
					f2.write('\n')
					mat = defaultdict(lambda: 1)
					mat [(0,'*','*')] = 1
					idx = 0
					continue
				word = line.strip()
				tags = []
				for u in possible_tags:
					for v in possible_tags:
						pi_n = {w:mat.get((idx,w,u),1)*self.compute_trigram(v,w,u)*self.compute_emission(word,w) for w in possible_tags}
						tag = max(pi_n, key=pi_n.get)
						cur_pi = pi_n[tag]
						mat[(idx+1,u,v)] = cur_pi
						tags.append((tag,cur_pi))
				# print tags
				argmax = max(tags, key=lambda x: x[1])
				f2.write('%s %s %s\n' % (word,argmax[0], ln(argmax[1])))
				idx+=1
				num_lines -= 1
				if num_lines == 0:
					return

if __name__ == "__main__":
	ln = lambda x: math.log(x) if x > 0 else 0
	t = Tagger()
	parser = optparse.OptionParser()
	parser.add_option("-i", "--interactive",
                  action="store_true", dest="interactive", default=False,
                  help="Run in interactive mode for trigram probabilities.")
	(options,args) = parser.parse_args()
	if options.interactive:
		interactive_log_prob()
	else:
		t.replace_rare('ner_train.dat',5)
		# write_tag_probabilities(True)
		# write_tag_probabilities(False)
		# test_viterbi(True)
		# test_viterbi(False)

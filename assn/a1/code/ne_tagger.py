# coding: utf-8
#!/usr/bin/python
__author__="Arvind Srinivasan <vs2371@columbia.edu>"
import re
import csv
import math
import itertools
import optparse
import shutil
from collections import defaultdict
from count_freqs import Hmm

ln = lambda x: math.log(x) if x > 0 else "-inf"


"""
random assertions that are verified as correct to avoid dumb mistakes
"""
def test_minimal():
	t = Tagger()
	assert t.compute_emission('University','I-ORG'), 0.001299870013
	t_rare = Tagger('ner_train-rare.dat')
	assert t_rare.compute_emission('University','I-ORG'), 0.00130039011704
	# assert t_rare.compute_emission('_RARE_')


"""
takes argmax of tag probabilities using the model specfied in Question 4 and writes results to q4_prediction_file and q4_prediction_file-rare.
"""
def write_tag_probabilities(rare=True):
	t = Tagger("ner_train-rare.dat") if rare else Tagger("ner_train.dat")
	outfile = "q4_prediction_file-rare" if rare else "q4_prediction_file"
	with open(outfile, "w") as f1, open("ner_dev.dat") as f:
		for line in f:
			if line != '\n':
				p = {tag: t.compute_emission(line.strip(),tag) for tag in t.unigrams}
				p = dict(filter(lambda t: t[1]!=0, p.iteritems()))
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

def test_viterbi(rare=False,minimal=False):
	t = Tagger("ner_train-rare.dat") if rare else Tagger("ner_train.dat")
	outfile = "viterbi_prediction_file-rare" if rare else "viterbi_prediction_file"
	filename = "dev-minimal" if minimal else "ner_dev.dat"
	t.viterbi(filename,outfile)

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
		self.words = [x[0] for x in self.counter.emission_counts.keys()]
	"""
	conditional probability that the word maps to tag given the number of times the tag occurs
	"""
	def compute_emission(self, word, tag):
		em = self.counter.emission_counts
		if tag == '*':
			return 0
		if (word,tag) in em:
			return em[(word,tag)]/float(self.unigrams[tag])
		elif word in self.words:
			return 0
		else:
			return em[('_RARE_',tag)]/float(self.unigrams[tag])

	def compute_trigram(self,yi,y1,y2):
		return self.trigrams.get((y2,y1,yi),0)/float(self.bigrams.get((y2,y1),1))

	"""
	basic file replacement, writes to a new file called rare-{infile} where infile is provided. Can pass a threshold of how many common_words
	is considered "rare"
	"""
	def replace_rare(self,infile,threshold=5):
		wordcounts = defaultdict(int)
		for tup in self.counter.emission_counts.iteritems():
			wordcounts[tup[0][0]] += tup[1] # aggregates counts of words total, with any tag
		print wordcounts
		common_words = [k for k,v in wordcounts.iteritems() if v >= threshold]
		replaced = 0
		f = open(infile)
		f2 = open(infile.replace('.dat','-rare.dat'), 'w')
		for line in f:
			if len(line.split(' ')) == 2:
				if line.split(' ')[0] not in common_words: # closed set, there are more rare than not rare, we know it's one or the other
					f2.write(line.replace(line.split(' ')[0], '_RARE_', 1))
					replaced +=1
				else:
					f2.write(line)
			else:
				f2.write(line) # maintain stops
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

	def viterbi(self,infile,outfile):
		
		def write_to_pred_file(f,sentence):
			tag_seq = [" ".join(x) for x in self.tag_sequence(sentence)]
			for word,tag in itertools.izip(sentence,tag_seq):
				f.write('%s %s\n' % (word,tag))
			f.write('\n')

		with open(infile) as f, open(outfile,"w") as f2:
			sentence = []
			for line in f:
				if line == '\n':
					# print list(self.tag_sequence(sentence))
					write_to_pred_file(f2,sentence)
					sentence = []
					continue
				else:
					sentence.append(line.strip())
			write_to_pred_file(f2,sentence)


	def tag_sequence(self,sentence):
		possible_tags = self.unigrams.keys()
		possible_tags.append('*')
		bp = {i:{} for i in range(len(sentence) + 1)}
		bp[0] = {t:('O',0) for t in itertools.product(possible_tags,repeat=2)}
		bp[0][('*','*')] = (1.0,1.0)
		for v in possible_tags:
			tag_max = ('sentinel',-1)
			tags = {}
			for w in possible_tags:
				# print "word: %s, v: %s, w: %s, q: %s, e: %s" % (sentence[0], v,w,self.compute_trigram(v,w,'*'), self.compute_emission(sentence[0],v))
				tags[w] = bp[0][(w,'*')][1]*self.compute_trigram(v,w,'*')*self.compute_emission(sentence[0],v)
				if tags[w] > tag_max[1] and tags[w] != 0:
					tag_max = (w,tags[w])
					print v, tag_max
			bp[1][('*',v)] = tag_max if tag_max != ('sentinel',-1) else ('O',0)
		# print bp[1]
		for i,word in enumerate(sentence[1:], start=2):
			for v,u in itertools.product(possible_tags,repeat=2):
				tag_max = ('sentinel', -1)
				tags = {}
				for w in possible_tags:
					# print "word: %s, u: %s, v: %s, w: %s, q: %s, e: %s" % (word, u,v,w,self.compute_trigram(v,w,u), self.compute_emission(word,v))
					if (w,u) in bp[i-1]:
						tags[w] = bp[i-1][(w,u)][1]*self.compute_trigram(v,u,w)*self.compute_emission(word,v)
						if tags[w] > tag_max[1] and tags[w] != 0:
							tag_max = (w,tags[w])
							print v,tag_max
					bp[i][(u,v)] = tag_max if tag_max != ('sentinel',-1) else ('O',0)
			print "\n\n"
		print bp[2]
		n = len(sentence)
		print n
		last = {(u,v): bp[n][(u,v)][1]*self.compute_trigram('STOP',v,u) for u,v in bp[n].keys()}
		print "\n\n"
		print last
		yn1,yn = max(last, key=last.get)
		conf = last[(yn1,yn)]
		seq = [(yn,str(ln(conf))), (yn1,str(ln(conf)))]

		for i in xrange(len(sentence) - 2, 0, -1):
			u,v = tuple(x[0] for x in reversed(seq[-2:]))
			prev = bp[i+2][(u,v)]
			seq.append((prev[0], str(ln(prev[1]))))
		return seq

if __name__ == "__main__":
	t = Tagger()
	parser = optparse.OptionParser()
	parser.add_option("-i", "--interactive",
                  action="store_true", dest="interactive", default=False,
                  help="Run in interactive mode for trigram probabilities.")

	parser.add_option("-m", "--minimal",
                  action="store_true", dest="minimal", default=False,
                  help="Run in minimal (testing) mode, which uses a small/faster corpus for viterbi")
	(options,args) = parser.parse_args()
	if options.interactive:
		interactive_log_prob()
	else:
		# t.replace_rare('ner_train.dat',5)
		# write_tag_probabilities(True)
		# write_tag_probabilities(False)
		test_viterbi(True,minimal=options.minimal)
		# test_viterbi(False)

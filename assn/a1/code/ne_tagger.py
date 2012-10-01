# coding: utf-8
#!/usr/bin/python
__author__="Arvind Srinivasan <vs2371@columbia.edu>"
import re
import math
import itertools
import optparse
from collections import defaultdict
from count_freqs import Hmm

ln = lambda x: math.log(x) if x > 0 else "-inf" # to avoid math.log must be a float errors -- should never use a "-inf" value, just useful for defaulting

"""
takes argmax of tag probabilities using the model specfied in Question 4 and writes results to q4_prediction_file and q4_prediction_file-rare.
"""
def write_tag_probabilities(rare=True,minimal=False):
	t = Tagger("ner_train-rare.dat") if rare else Tagger("ner_train.dat")
	outfile = "q4_prediction_file-rare" if rare else "q4_prediction_file"
	eval_file = "dev-minimal" if minimal else "ner_dev.dat" 
	with open(outfile, "w") as f1, open(eval_file) as f:
		for line in f:
			if line != '\n':
				p = {tag: t.compute_emission(line.strip(),tag) for tag in t.unigrams} #where t.unigrams should have all the tags
				p = dict(filter(lambda t: t[1]!=0, p.iteritems()))
				max_prob = max(p, key=p.get)
				# can't take log of 0, so defaults
				f1.write("%s %s %s\n" % (line.strip(), max_prob, ln(p[max_prob])))
			else:
				f1.write(line)
"""
reads constantly from the command line, and prints the q(trigram) values. For 5a, but also for debugging.
"""
def interactive_log_prob():
	t = Tagger()
	while True:
		try:
			trigram = raw_input("\n Enter the trigram - yi-2, yi-1, yi separated by spaces: ").split(' ')
			trigram.reverse() #reversed because the function takes yi, yi-1, yi-2
			print "Log Probability: %s" % ln(t.compute_trigram(*trigram))
		except EOFError: # exit with ctrl+d, or kill with ctrl+c as usual
			break
"""
wrapper function for viterbi calls that binds to a file. I wanted the program to run in a modular way instead
of piping to files & printing to stdout, because that way I could print debug statements without having to write to stderr
and piping every time I ran the fn.
"""
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
		self.unigrams = {k[0]:v for k,v in self.counter.ngram_counts[0].iteritems()} #since the key is a one-word tuple
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
	"""
	returns the trigram count over the bigram count, defaulting the dict gets so that there aren't division by 0 errors
	"""
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
	"""
	returns a dictionary of relative probabilities for emission counts
	"""
	def tag_probabilities(self,word):
		counts = {tag:self.compute_emission(word,tag) for tag in self.unigrams}
		prob = lambda v: v/sum(counts.values()) if sum(counts.values()) != 0 else 0
		return {k:prob(v) for k,v in counts.iteritems()}
	"""
	wrapper function for dynamic programming algorithm, writes to outfile
	"""
	def viterbi(self,infile,outfile):
		
		def write_to_pred_file(f,sentence):
			tag_seq = [" ".join(x) for x in self.tag_sequence(sentence)] #tuples of tag,probability
			for word,tag in itertools.izip(sentence,tag_seq): # word, tag, probability
				f.write('%s %s\n' % (word,tag))
			f.write('\n')

		with open(infile) as f, open(outfile,"w") as f2:
			sentence = []
			for line in f:
				if line == '\n':
					write_to_pred_file(f2,sentence)
					sentence = []
					continue
				else:
					sentence.append(line.strip())
			#write the last sentence to the file (if there is no newline -- will just return and escape if sentence is empty
			write_to_pred_file(f2,sentence) 


	def tag_sequence(self,sentence):
		if len(sentence) == 0:
			return []
		possible_tags = self.unigrams.keys()
		possible_tags.append('*')
		bp = {i:{} for i in range(len(sentence) + 1)}
		# initialization: pi(0,'*','*') = 1, pi(0,u,v) = 0
		bp[0] = {t:('O',0) for t in itertools.product(possible_tags,repeat=2)}
		bp[0][('*','*')] = (1.0,1.0)
		# at idx 1, u can only be *
		for v in possible_tags:
			tag_max = ('sentinel',-1) #a real probability (since logs are only computed at end) will never be negative, so this will be reset
			tags = {}
			for w in possible_tags:
				tags[w] = bp[0][(w,'*')][1]*self.compute_trigram(v,w,'*')*self.compute_emission(sentence[0],v)
				if tags[w] > tag_max[1] and tags[w] != 0:
					tag_max = (w,tags[w])
			bp[1][('*',v)] = tag_max if tag_max != ('sentinel',-1) else ('O',0) #default tag is no tag, so O -- no sequences with this u,v with a nonzero probability

		for i,word in enumerate(sentence[1:], start=2): #from 2...n
			for v,u in itertools.product(possible_tags,repeat=2): #same as nested for u in K, v in K
				tag_max = ('sentinel', -1)
				tags = {}
				for w in possible_tags:
					if (w,u) in bp[i-1]:
						tags[w] = bp[i-1][(w,u)][1]*self.compute_trigram(v,u,w)*self.compute_emission(word,v)
						if tags[w] > tag_max[1] and tags[w] != 0:
							tag_max = (w,tags[w])
					bp[i][(u,v)] = tag_max if tag_max != ('sentinel',-1) else ('O',0)

		n = len(sentence)
		last = {(u,v): bp[n][(u,v)][1]*self.compute_trigram('STOP',v,u) for u,v in bp[n].keys()}
		yn1,yn = max(last, key=last.get) # max probability for sequence ending in STOP
		conf = last[(yn1,yn)]
		seq = [(yn,str(ln(conf))), (yn1,str(ln(conf)))] #sequence will be yn...y0

		for i in xrange(len(sentence) - 2, 0, -1):
			u,v = tuple(x[0] for x in reversed(seq[-2:])) #previous two are yn-1, yn-2
			prev = bp[i+2][(u,v)]
			seq.append((prev[0], str(ln(prev[1]))))
		return reversed(seq) #reversed yn...y0 is y0...yn

if __name__ == "__main__":

	parser = optparse.OptionParser()
	parser.add_option("-i", "--interactive",
                  action="store_true", dest="interactive", default=False,
                  help="Run in interactive mode for trigram probabilities.")
	parser.add_option("-r", "--replace",
                  action="store_true", dest="replace", default=False,
                  help="Replace all the words with frequency < 5 with _RARE_ in the dataset")
	parser.add_option("-v", "--viterbi",
                  action="store_true", dest="viterbi", default=False,
                  help="Test the viterbi tagger and output to viterbi_prediction_file-rare")
	parser.add_option("-b", "--baseline",
                  action="store_true", dest="baseline", default=False,
                  help="Run the baseline tagger and output to q4_prediction_file-rare")
	parser.add_option("-m", "--minimal",
                  action="store_true", dest="minimal", default=False,
                  help="Run in minimal (testing) mode, which uses a small/faster corpus")

	#runtime options
	(options,args) = parser.parse_args()
	if options.interactive:
		interactive_log_prob()
	elif options.baseline:
		write_tag_probabilities(True,minimal=options.minimal)
	elif options.replace:
		t = Tagger()
		t.replace_rare('ner_train.dat',5)
	elif options.viterbi:
		test_viterbi(True,minimal=options.minimal)

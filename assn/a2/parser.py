import json
from subprocess import *
from collections import defaultdict
from count_cfg_freq import Counts

class Parser:
    def __init__(self, filename):
        self.counter = Counts()
        for l in open(filename):
            self.counter.count(json.loads(l))

    def replace_rare(self):
        vocabulary = defaultdict(lambda: 0)
        for tup,count in self.counter.unary.iteritems():
            nonterm,word = tup
            vocabulary[word] += count
        rare_words = set(map(lambda x: x[0], filter(lambda c: c[1] < 5, vocabulary.iteritems())))
        awk_replace(rare_words,chunk=100)
        set_replace(rare_words)

        def awk_replace(rare_words, chunk, infile, outfile):
            Popen(['chmod','+x','awk_replace_rare.sh'])
            #didn't want to buffer in memory, which is a standard problem with popen, so had a helper script that piped through unix
            def sub (re,infile,outfile):
                p1 = Popen(['./awk_replace_rare.sh', infile, outfile, re])
                p1.wait()

            escaped_words = [x.replace('/,\/') for x in words]
            for i in xrange(0,len(escaped_words), chunk):
                regexp = "|".join(escaped_words[i:i+chunk])
                sub(regexp, infile,outfile)

        def set_replace(rare_words,infile,outfile):
            with open(infile) as f, open(outfile) as f2:
                for line in f:
                    line_words = set(line.translate(None,'[,"]\n').split()) #strip json characters
                    rare_in_line = rare_words & line_words
                    for w in rare_in_line:
                        f2.write(line.replace(w,'_RARE_'))
                


import json
from subprocess import *
from collections import defaultdict
from count_cfg_freq import Counts

class Parser:
    def __init__(self, filename):
        self.filename = filename
        self.counter = Counts()
        for l in open(filename):
            self.counter.count(json.loads(l))

    def replace_rare(self):

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
            with open(infile) as f, open(outfile,'w') as f2:
                for line in f:
                    line_words = set(line.translate(None,'[,"]\n').split()) #strip json characters
                    rare_in_line = rare_words & line_words
                    if len(rare_in_line) > 0:
                        print rare_in_line
                        l = line.replace(rare_in_line.pop(),'_RARE_')
                        for w in rare_in_line:
                            l = l.replace(w,'_RARE_')
                        f2.write(l)
                    else:
                        f2.write(line)

        vocabulary = defaultdict(lambda: 0)
        for tup,count in self.counter.unary.iteritems():
            nonterm,word = tup
            vocabulary[word] += count
        rare_words = set(map(lambda x: x[0], filter(lambda c: c[1] < 5, vocabulary.iteritems()))) - set(self.counter.nonterm.keys())
        set_replace(rare_words, self.filename, self.filename + "-rare")
    


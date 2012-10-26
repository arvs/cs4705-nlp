import json
import itertools
from subprocess import *
from collections import defaultdict
from count_cfg_freq import Counts

class Parser:
    Rule = namedtuple('Rule',['parent','left','right'])
    terminal = namedtuple('terminal',['parent','word'])
    bp = namedtuple('bp',['rule','s'])

    def __init__(self, filename):
        self.filename = filename
        self.counter = Counts()
        for l in open(filename):
            self.counter.count(json.loads(l))
        self.rules = defaultdict(list)
        for tup in self.counter.binary.keys():
            rules[tup[0]].append(Rule(*tup))

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
                    rare_in_line = rare_words & line_words #intersection of rare and in line
                    if len(rare_in_line) > 0:
                        l = line.replace('"%s"' % rare_in_line.pop(),'"_RARE_"')
                        for w in rare_in_line:
                            l = l.replace('"%s"' % w,'"_RARE_"')
                        f2.write(l)
                    else:
                        f2.write(line)

        vocabulary = defaultdict(lambda: 0)
        for tup,count in self.counter.unary.iteritems():
            nonterm,word = tup
            vocabulary[word] += count
        self.rare_words = set(map(lambda x: x[0], filter(lambda c: c[1] < 5, vocabulary.iteritems()))) - set(self.counter.nonterm.keys())
        set_replace(rare_words, self.filename, + "rare-" + self.filename)

    def q(self, parent, children=None):
        count_nonterm = self.counter.nonterm.get(parent)
        if count_nonterm is None:
            return 0
        if isinstance(children, str):
            rule_count = self.counter.unary.get((parent, children[0]), self.counter.unary.get((parent, '_RARE_'), 0))
        elif len(children) == 2:
            rule_count = self.counter.binary.get((parent, children[0], children[1]), 0)
        return float(rule_count)/count_nonterm

    def parse(self,filename):
        with open(filename) as f:
            for sentence in f:
                max_tree = self.cky_recover(self.cky(line.split(' ')))
                print json.dumps(max_tree)

    def cky(self,sentence):

        def cky_recover(pi,left,right,parent):
            if left == right:
                return [parent, pi[(left,right,parent)][2][1]]
            else:
                split = pi[(left,right,parent)][1]
                print left, right, split
                return [parent, cky_recover(pi,left,split,parent), cky_recover(pi,split+1,right,parent)]

        pi = {}

        n = len(sentence)
        for idx,word in enumerate(sentence,1):
            for X in self.r.keys():
                pi[(idx,idx,X)] = (self.q(X,word),bp(terminal(X,word), None))

        for l in xrange(1, n):
            for i in xrange(1, n + 1 - l):
                j = i + l
                max_prob, split, rule = (-1,-1,'sentinel')
                # flatten, so that it is one large loop instead of many
                for X, possible_rules in self.rules()
                    for rule in possible_rules:
                        for s in xrange(i,j):
                            prob = self.q(X, [rule.left, rule.right])* \
                                pi.get((i,s,ch1),[0])[0]* \
                                pi.get((j,s+1,ch2),[0])[0]
                            if prob > max_prob:
                                max_prob = (prob,s,(X,ch1,ch2))
                            pi[(i,j,X)] = (max_prob,split,rule)

        head = pi.get((1,n,'S'))
        import ipdb
        ipdb.set_trace()
        if head is None:
            candidates = {t: t[1][0] for t in filter(lambda tup: tup[0][:2] == (1,n), pi.iteritems())}
            head = max(candidates, key=candidates.get)

        p, split, rule = head
        tree = [rule[0], cky_recover(pi,1,split, rule[0]), cky_recover(pi,split+1,n,rule[0])]
        return tree

if __name__ == '__main__':
    p = Parser('../train/parse_train.dat-rare')
    a = "The complicated language in the huge new law has muddied the fight ."
    print p.cky(a.split())
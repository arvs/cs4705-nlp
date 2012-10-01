Note: all of the files submitted are trained with the latest version of the tagger, so eval can be run directly, but you can either see the makefile or below to rerun and overwrite the files. If you don't have make, just run the commands in the makefile directly in the directory.
Q4) run "make replace" and then "make baseline" or "make baseline-minimal"
========
With _RARE_ replacement (rare < 5)
========

Found 14043 NEs. Expected 5931 NEs; Correct: 3117.

		 precision 	recall 		F1-Score
Total:	 0.221961	0.525544	0.312106
PER:	 0.435451	0.231230	0.302061
ORG:	 0.475936	0.399103	0.434146
LOC:	 0.147750	0.870229	0.252612
MISC:	 0.491689	0.610206	0.544574

========
With _RARE_ replacement (rare < 3)
========

Found 10762 NEs. Expected 5931 NEs; Correct: 4116.

	 	 precision 	recall 		F1-Score
Total:	 0.382457	0.693981	0.493141
PER:	 0.233288	0.844940	0.365627
ORG:	 0.498061	0.479821	0.488770
LOC:	 0.809407	0.722465	0.763469
MISC:	 0.505513	0.647123	0.567619

========
With _RARE_ replacement (rare < 8)
========

		 precision 	recall 		F1-Score
Total:	 0.187166	0.484910	0.270085
PER:	 0.388664	0.156692	0.223342
ORG:	 0.076391	0.692825	0.137609
LOC:	 0.805556	0.632497	0.708613
MISC:	 0.477143	0.543974	0.508371

- This output makes sense. In a unigram model, the emission parameters for rare words will have some constant argmax for tag probability (in this case LOC), so it has a lot of false positives, and misses out on any of the rare words that are tagged with something other than LOC.

- When we decrease the threshold of what we call "rare," the overall accuracy goes up mainly because the number of false positives go down. This is again because rare words are always tagged as  I-LOC. 

- When we increase the rare threshold, the accuracy goes significantly down. This is probably because there are words in the dev dataset that are also in the training corpus, but at a low frequency, so the emission probabilities are skewed.


Q5) a) run "make interactive"
	b) run "make viterbi" or "make viterbi-minimal" for a smaller test set. To change the keyfile, look at the test_viterbi method in the tagger. 
======
eval
======

Found 4702 NEs. Expected 5931 NEs; Correct: 3645.

	 	 precision 	recall 		F1-Score
Total:	 0.775202	0.614568	0.685601
PER:	 0.762535	0.595756	0.668907
ORG:	 0.611855	0.478326	0.536913
LOC:	 0.876289	0.695202	0.775312
MISC:	 0.830065	0.689468	0.753262

Observations:
- The tagger tends to produce false negatives rather than false positives.

Q6) Things that I would implement if I had time:
The overarching theme of the inaccuracy is that the emission counts are fairly arbitrary for a closed dataset, and could probably be improved by at least 10-15 percent with heuristics.
- A lot of time, I-ORGs are fuzzy substrings of I-LOCS. This is a specific example of how lexical similarity can be taken into account in emission counts.
- Another thing that could be incorporated into the emission count is capitalization - a first letter capitalized is probably indicative of a LOC or a Person, while all caps, especially separated by periods, is often an ORG.
- Some closed sets {South, North, East, West}, {punctuation} can be hardcoded to improve tagging, especially of LOCs and MISCs.


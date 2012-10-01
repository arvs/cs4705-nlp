viterbi:
	python ne_tagger.py -v && python eval_ne_tagger.py ner_dev.key viterbi_prediction_file-rare

viterbi-minimal:
	python ne_tagger.py -m -v && python eval_ne_tagger.py key-minimal viterbi_prediction_file-rare

baseline:
	python ne_tagger.py -b && python eval_ne_tagger.py ner_dev.key q4_prediction_file-rare

baseline-minimal:
	python ne_tagger.py -b -m && python eval_ne_tagger.py key-minimal q4_prediction_file-rare

replace:
	python ne_tagger.py -r

interactive:
	python ne_tagger.py -i
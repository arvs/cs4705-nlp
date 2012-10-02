* Goals of NLP *
	- Machine Translation
		- advance of statistical models to learn automated translations (learn from manual translations)
	- Information extraction
		- take raw unstructured text --> extract targets
			- map document collection to structured databases
			- search
			- statistical queries (how has x changed over y years)
	- Summarization
	- Dialogue systems
		- build tree of responses with fuzzy recognition (incl spoken)
			- siri

* Canonical Problems *
	- Strings to tagged sequences
		- ex: part of speech tagging : Take a sentence and tag with parts of speech
		- ex: named entity tagging : take sentence and extract locations, companies
	- Parsing
		- ex: take sentence as input, produce morphological parse tree
		- parse tree groupings are huge source of ambiguity (phrases, colloquial, etc)
	- Acoustic recognition:
		- eliminating ambiguity from speech recognition
	- Homophonic ambiguity
		- multiple meanings for words
		- pronoun ambiguity --> choosing from references in context

* Language Modeling Problem *
	- finite set of words
	- infinite set of strings
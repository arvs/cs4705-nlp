#!/usr/bin/perl

# The output of the parser should be processed by this script, before 
# being submitted to the evalb program.
#
# The reason for this: evalb has the following problem (bug?). Words
# are disregarded in scoring if they are tagged as a piece of punctuation.
# If the parser (or underlying part-of-speech tagger) makes an error,
# and tags a "real word" as punctuation, or vice versa, this will lead
# to a misalignment with the gold file in the evalb script, and the
# sentences with this problem will cause errors in evalb. Perhaps a
# better way for evalb to operate would be to base the punctuation
# decision on the gold-standard tags alone: this would guarantee
# alignment between the gold-standard and the parser's output.
#
# This problem actually occurs on two sentences in the POS tagged file
# for section 23. In sentence 10, "-LCB-" is tagged as a comma rather
# than "-LRB-", in sentence 1962, "'" is tagged as "POS" instead of "''"
#
# A better solution might be to correct the evalb program. But for
# now this perl script provides a fix to section 23 files with this problem.
# In addition to having the advantage of being a quicker (albeit
# file-specific) fix, I thought this would be more transparent than
# making alterations to the evalb script, in case anybody would like to
# understand the corrections.

$n = 0;

while(<>)
{
    $n++;
    if($n==10)
    {
	if(!m/Once.*again.*-LCB-.*the.*specialists.*-RCB-.*were.*not.*able.*to.*handle.*the.*imbalances.*on.*the.*floor.*of.*the.*New.*York.*Stock.*Exchange.*,.*''.*said.*Christopher.*Pedersen.*,.*senior.*vice.*president.*at.*Twenty-First.*Securities.*Corp/)
	{
	    die "Error 1 in section 23 file";
	}
	s/\(, -LCB-\)/\(-LRB- -LCB-\)/g;
    }

    if($n==1962)
    {
	if(!m/The.*only.*thing.*you.*do.*n't.*have.*,.*''.*he.*said.*,.*``.*is.*the.*`.*portfolio.*insurance.*'.*phenomenon.*overlaid.*on.*the.*rest/)
	{
	    die "Error 2 in section 23 file";
	}
	s/\(POS '\)/\(`` '\)/g;
    }
    print;
}

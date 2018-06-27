#!/usr/bin/bash

if [ $# != 1 ]; then
    echo "Usage: char.sh word_file"
    exit -1;
fi

cat $1 | perl -CSDA -ane '
{
    print $F[0];
    foreach $s (@F[1..$#F]) {
        if (($s =~ /\[.*\]/) || ($s =~ /\<.*\>/) || ($s =~ "!SIL")) {
            print " $s";
        } else {
            @chars = split "", $s;
            foreach $c (@chars) {
                print " $c";
            }
        }
    }
    print "\n";
}'

#!/usr/bin/perl
use strict;
use warnings;

my $server="http://127.0.0.1:3121/";
my $curl="/usr/local/bin/curl";
my $method=$ARGV[0];
print("Request:\t$method\n");
system("$curl --data-binary '$method' -H 'content-type: text/plain;' $server");
exit(0);

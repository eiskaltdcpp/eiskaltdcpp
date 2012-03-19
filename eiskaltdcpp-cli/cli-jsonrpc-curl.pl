#!/usr/bin/perl
#
# Copyright (c) 2012 Dmitry Kolosov <onyx@z-up.ru>
#
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
#	1. Redistributions of source code must retain the above copyright notice,
#	this list of conditions and the following disclaimer.
#	2. Redistributions in binary form must reproduce the above copyright 
#	notice, this list of conditions and the following disclaimer in the 
#	documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
#
use strict;
use warnings;

my $server="http://127.0.0.1:3121/";
my $curl="/usr/local/bin/curl";
my $method=$ARGV[0];
print("Request:\t$method\n");
print("Reply:\t");
system("$curl --data-binary '$method' -H 'content-type: text/plain;' $server");
exit(0);

#!/usr/bin/perl
#
#Copyright (c) 2011, Dmitry Kolosov
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, 
#are permitted provided that the following conditions are met:
#	1. Redistributions of source code must retain the above copyright notice,
#	this list of conditions and the following disclaimer.
#	2. Redistributions in binary form must reproduce the above copyright 
#	notice, this list of conditions and the following disclaimer in the 
#	documentation and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
#INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
#POSSIBILITY OF SUCH DAMAGE.

# using Eiskaltdcpp core 2.2 xml-rpc
# onyx@z-up.ru

use warnings;
use strict;
use RPC::XML::Client;
use RPC::XML::Parser;
use Term::ShellUI;
use Cwd;

# preparing terminal
binmode STDOUT, ':utf8';

# configuration
our %config;
require "config.pl";
print("Configuration:\n");
foreach (keys %config)
{
	print("$_:\t$config{$_}\n");
}

my $P = RPC::XML::Parser->new();
my $cli = RPC::XML::Client->new($config{eiskaltURL}) or die "Can not connect to hub url $config{eiskaltURL}: $_\n";
my $term = new Term::ShellUI(commands => get_commands(), history_file => $config{hist_file}, history_max => $config{hist_max});
$term->prompt("$config{eiskaltHostPort} $config{prompt}");
$term->run();

sub get_commands
{
	return 
	{

		"help" => 
		{ 
			desc => "Print helpful information",
			args => sub { shift->help_args(undef, @_); },
			method => sub { shift->help_call(undef, @_); } 
		},
		"hub.add" =>
		{
			desc => "Add a new hub and connect. Parameters: s/'huburl', s/'encoding'",
			minargs => 2,
			maxargs => 2,
			proc => \&hubadd
		},
		"hub.del" =>
		{
			desc => "Disconnect hub and delete from autoconnect. Parameters: s/'huburl'",
			# add autocomplete from list of connected hubs
			minargs => 1,
			maxargs => 1,
			proc => \&hubdel

		},
		"hub.say" =>
		{
			desc => "Send a public message to hub. Parameters: s/'huburl', s/'message'",
			# add autocomplete from list of connected hubs 
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&hubsay
		},
		"hub.pm" =>
		{
			desc => "Send a private message to nick on hub. Parameters: s/'huburl', s/'nick', s/'message'",
			# add autocomplete from connected hubs and nicks
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 3,
			maxargs => 3,
			proc => \&hubpm

		},
		"hub.list" =>
		{
			desc => "Get a list of hubs, configured to connect to. Parameters: s/'separator'",
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			#minargs => 1,
			maxargs => 1,
			proc => \&hublist
		},
		"hub.retchat" =>
		{
			desc => "Get last public chat messages. Parameters: s/'huburl', s/'separator'",
			minargs => 1,
			maxargs => 2,
			proc => \&hubretchat
		},
		"daemon.stop" =>
		{
			desc => "Disconnect from hubs and exit. Parameters: i/1",
			minargs => 1,
			maxargs => 1,
			proc => \&daemonstop

		},
		"magnet.add" =>
		{
			desc => "Add a magnet to download queue, and fetch it to download directory. Parameters: s/'magnet', s/'download directory'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&magnetadd
		},
		"share.add" =>
		{
			desc => "Add a directory to share as virtual name. Parameters: s/'directory',s/'virtual name'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&shareadd
		},
		"share.rename" =>
		{
			desc => "Give a new virtual name to shared directory. Parameters: s/'directory',s/'virtual name'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&sharerename
		},
		"share.del" =>
		{
			desc => "Unshare directory with virtual name. Parameters: Parameters: s/'virtual name'",
			minargs => 1,
			maxargs => 1,
			proc => \&sharedel
		},
		"share.list" =>
		{
			desc => "Get a list of shared directories. Parameters: s/'separator'",
			maxargs => 1,
			proc => \&sharelist
		},
		"share.refresh" =>
		{
			desc => "Refresh a share, hash new files. Parameters: i/1",
			minargs => 1,
			maxargs => 1,
			proc => \&sharerefresh
		},
		"list.download" =>
		{
			desc => "Download a file list from nick on huburl. Parameters: s/'huburl', s/'nick'",
			minargs => 2,
			maxargs => 2,
			proc => \&listdownload
		},
		"search.send" =>
		{
			desc => "Start hub search. Parameters: s/'search string'",
			minargs => 1,
			maxargs => 1,
			proc => \&searchsend
		},
		"search.list" =>
		{
			desc => "Get search results. Parameters: s/'index of search', s/'huburls'",
			minargs => 2,
			maxargs => 2,
			proc => \&searchlist
		},
		"search.retresults" => 
		{
			desc => "Not implemented"
		},
		"prompt" =>
		{
			desc => "Set custom prompt",
			minargs => 1,
			maxargs =>1,
			proc => sub { $term->prompt("$config{eiskaltHostPort} ".shift) }
		},
		"exec" =>
		{
			desc => "Execute shell command",
			args => sub { shift->complete_files(@_); },
			minargs => 1,
			proc => sub { system(shift) }
		},
		"quit" => 
		{
			desc => "Quit this program", 
			maxargs => 0,
			method => sub { shift->exit_requested(1); }
		},
		"exit" => { alias => 'quit' },
		'' =>
		{
	  		proc => "No command here by that name\n",
			desc => "No help for unknown commands."
		}
	}
}

# functions
sub hubadd($$)
{
	my $req=RPC::XML::request->new('hub.add', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub hubdel($)
{
	my $req=RPC::XML::request->new('hub.del', RPC::XML::string->new($_[0]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub hubsay($$)
{
	my $req=RPC::XML::request->new('hub.say', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub hubpm($$$)
{
	my $req=RPC::XML::request->new('hub.pm', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]), RPC::XML::string->new($_[2]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub hublist($)
{
	my $req=RPC::XML::request->new('hub.list', RPC::XML::string->new($_[0] || $config{separator}));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub hubretchat($$)
{
	my $req=RPC::XML::request->new('hub.retchat', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1] || $config{separator}));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub daemonstop($)
{
	my $req=RPC::XML::request->new('daemon.stop', RPC::XML::int->new($_[0]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub magnetadd($$)
{
	my $req=RPC::XML::request->new('magnet.add', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub shareadd($$)
{
	my $req=RPC::XML::request->new('share.add', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub sharerename($$)
{
	my $req=RPC::XML::request->new('share.rename', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub sharedel($)
{
	my $req=RPC::XML::request->new('share.del', RPC::XML::string->new($_[0]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub sharelist($)
{
	if (defined($_[0])) {print('>',$_[0],'<')}
	my $req=RPC::XML::request->new('share.list', RPC::XML::string->new($_[0] || $config{separator}));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub sharerefresh($)
{
	my $req=RPC::XML::request->new('share.refresh', RPC::XML::int->new($_[0]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub listdownload($$)
{
	my $req=RPC::XML::request->new('list.download', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub searchsend($)
{
	my $req=RPC::XML::request->new('search.send', RPC::XML::string->new($_[0]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub searchlist($$)
{
	my $req=RPC::XML::request->new('search.list', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1]));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}





__END__


=pod

			#desc => "Start hub search. Parameters: s/'search string', i/type, i/sizemode, i/sizetype, d/size, s/'huburls'",
			#doc => qq[Start hub search. Parameters: s/'search string', i/type, i/sizemode, i/sizetype, d/size, s/'huburls'
	
#type is an integer, one of the following:
#"2" is for search 7z ,ace ,arj ,bz2 ,gz ,lha ,lzh ,rar ,tar ,z ,zip files
#"1" is for search ape ,flac ,m4a ,mid ,mp3 ,mpc ,ogg ,ra ,wav ,wma files
#"4" is for search app ,bat ,cmd ,com ,dll ,exe ,jar ,msi ,ps1 ,vbs ,wsf files
#"3" is for search doc ,docx ,htm ,html ,nfo ,odf ,odp ,ods ,odt ,pdf ,ppt ,pptx ,rtf ,txt ,xls ,xlsx ,xml ,xps files
#"6" is for search 3gp ,asf ,asx ,avi ,divx ,flv ,mkv ,mov ,mp4 ,mpeg ,mpg ,ogm ,pxp ,qt ,rm ,rmvb ,swf ,vob ,webm ,wmv files
#"5" is for search bmp ,cdr ,eps ,gif ,ico ,img ,jpeg ,jpg ,png ,ps ,psd ,sfw ,tga ,tif ,webp files
#"7" is for search iso ,mdf ,mds ,nrg ,vcd ,bwt ,ccd ,cdi ,pdi ,cue ,isz ,img ,vc4 files

#sizemode and sizetype is commonly 0, size is commonly 0.0 (decimal value)],
			#minargs => 2,
			#maxargs => 6,
			#proc => sub {}



args
maxargs
minargs
proc
method

        "cd" => { desc => "Change to directory DIR",
                      args => sub { shift->complete_onlydirs(@_); },
                      maxargs => 1,
                      proc => sub { chdir($_[0] || $ENV{HOME} || $ENV{LOGDIR}) or print("Could not cd: $!\n") } },
        "delete" => { desc => "Delete FILEs",
                      args => sub { shift->complete_onlyfiles(@_); },
                      minargs => 1,
                      proc => sub { danger() && (unlink(@_) or warn "Could not delete: $!\n") } },
        "?" => { syn => "help" },
        "help" => { desc => "Print helpful information",
                      args => sub { shift->help_args(undef, @_); },
                      method => sub { shift->help_call(undef, @_); } },
        "ls" => { syn => "list" },
        "dir" => { syn => "ls" },
        "list" => { desc => "List files in DIRs",
                      args => sub { shift->complete_onlydirs(@_); },
                      proc => sub { system('ls', '-FClg', @_); } },
        "pwd" => { desc => "Print the current working directory",
                      maxargs => 0,
                      proc => sub { system('pwd'); } },
        "quit" => { desc => "Quit using Fileman",
                      maxargs => 0,
                      method => sub { shift->exit_requested(1); } },
        "rename" => { desc => "Rename FILE to NEWNAME",
                      args => sub { shift->complete_files(@_); },
                      minargs => 2, maxargs => 2,
                      proc => sub { danger() && system('mv', @_); } },
        "stat" => { desc => "Print out statistics on FILEs",
                      args => sub { shift->complete_files(@_); },
                      proc => \&print_stats },
        "cat" => { syn => "view" },
        "view" => { desc => "View the contents of FILEs",
                      args => sub { shift->complete_onlyfiles(@_); },
                      proc => sub { system('cat', @_); } },
# these demonstrate how parts of Term::ShellUI work:
        echo => { desc => "Echoes the command-line arguments",
                      proc => sub { print join(" ", @_), "\n"; } },
        tok => { desc => "Print command-line arguments with clear separations",
                      proc => sub { print "<" . join(">, <", @_), ">\n"; } },
        debug_complete => { desc => "Turn on completion debugging",
                      minargs => 1, maxargs => 1, args => "0=off 1=some, 2=more, 3=tons",
                      proc => sub { $term->{debug_complete} = $_[0] },
			},
=cut


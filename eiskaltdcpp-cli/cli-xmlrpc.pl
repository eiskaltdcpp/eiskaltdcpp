#!/usr/bin/perl
#
# Copyright (c) 2011 Dmitry Kolosov <onyx@z-up.ru>
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
# POSSIBILITY OF SUCH DAMAGE.

use warnings;
no warnings qw( uninitialized );
use strict;
use RPC::XML::Client;
use RPC::XML::Parser;
use Term::ShellUI;
use Cwd;
use Getopt::Long;
use Env qw[$XDG_CONFIG_HOME];

# use non-standart paths
BEGIN {
    unshift @INC, 
	 "$XDG_CONFIG_HOME/eiskaltdc++",
	 "~/.config/eiskaltdc++/",
	 "/usr/local/share/eiskaltdcpp/cli",
	 "/usr/share/eiskaltdcpp/cli"
}

# preparing terminal
binmode STDOUT, ':utf8';

# configuration
our %config;
$config{version}=0.2;
require "cli-xmlrpc-config.pl";
my $version,my $help;
GetOptions ('v|version' => \$version, 'h|help' => \$help);
if ($version)
{
	print("Command line interface version: $config{version}\n");
	exit(1);
}
if ($help)
{
	print(  "Using:
\teiskaltdcpp-cli
\teiskaltdcpp-cli <Key>
This is command line interface for eiskaltdcpp-daemon written on perl.
EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.

Keys:
\t-h, --help\t Show this message
\t-v, --version\t Show version string\n");
	exit(1);
}
print("Configuration:\n");
foreach (keys %config)
{
	print("$_: $config{$_}\n");
}

my $P = RPC::XML::Parser->new();
my $cli = RPC::XML::Client->new($config{eiskaltURL}, useragent => ['ssl_opts' => {verify_hostname => 0}]) or die "Can not connect to hub url $config{eiskaltURL}: $_\n";
$cli->credentials("Eiskaltdcpp XML-RPC Management",$config{user},$config{password});
my $term = new Term::ShellUI(commands => get_commands(), history_file => $config{hist_file}, history_max => $config{hist_max});
$term->prompt("$config{eiskaltHostPort} $config{prompt}");
$term->run();

sub get_commands
{
	return 
	{
		# v0.2
		"help" => 
		{ 
			desc => "Print helpful information",
			args => sub { shift->help_args(undef, @_); },
			method => sub { shift->help_call(undef, @_); } 
		},
		# v0.2
		"hub.add" =>
		{
			desc => "Add a new hub and connect. Parameters: s/'huburl', s/'encoding'",
			minargs => 2,
			maxargs => 2,
			proc => \&hubadd
		},
		# v0.2
		"hub.del" =>
		{
			desc => "Disconnect hub and delete from autoconnect. Parameters: s/'huburl'",
			# add autocomplete from list of connected hubs
			minargs => 1,
			maxargs => 1,
			proc => \&hubdel

		},
		# v0.2
		"hub.say" =>
		{
			desc => "Send a public message to hub. Parameters: s/'huburl', s/'message'",
			# add autocomplete from list of connected hubs 
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&hubsay
		},
		# v0.2
		"hub.pm" =>
		{
			desc => "Send a private message to nick on hub. Parameters: s/'huburl', s/'nick', s/'message'",
			# add autocomplete from connected hubs and nicks
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 3,
			maxargs => 3,
			proc => \&hubpm

		},
		# v0.2
		"hub.list" =>
		{
			desc => "Get a list of hubs, configured to connect to. Parameters: s/'separator'",
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			#minargs => 1,
			maxargs => 1,
			proc => \&hublist
		},
		"hub.getchat" =>
		{
			desc => "Get last public chat messages. Parameters: s/'huburl', s/'separator'",
			minargs => 1,
			maxargs => 2,
			proc => \&hubgetchat
		},
		# v0.2
		"daemon.stop" =>
		{
			desc => "Disconnect from hubs and exit. Parameters: none",
			proc => \&daemonstop
		},
		# v0.2
		"magnet.add" =>
		{
			desc => "Add a magnet to download queue, and fetch it to download directory. Parameters: s/'magnet', s/'download directory'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&magnetadd
		},
		# v0.2
		"share.add" =>
		{
			desc => "Add a directory to share as virtual name. Parameters: s/'directory',s/'virtual name'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&shareadd
		},
		# v0.2
		"share.rename" =>
		{
			desc => "Give a new virtual name to shared directory. Parameters: s/'directory',s/'virtual name'",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&sharerename
		},
		# v0.2
		"share.del" =>
		{
			desc => "Unshare directory with virtual name. Parameters: Parameters: s/'virtual name'",
			minargs => 1,
			maxargs => 1,
			proc => \&sharedel
		},
		# v0.2
		"share.list" =>
		{
			desc => "Get a list of shared directories. Parameters: s/'separator'",
			maxargs => 1,
			proc => \&sharelist
		},
		# v0.2
		"share.refresh" =>
		{
			desc => "Refresh a share, hash new files. Parameters: none",
			proc => \&sharerefresh
		},
		# v0.2
		"list.download" =>
		{
			desc => "Download a file list from nick on huburl. Parameters: s/'huburl', s/'nick'",
			minargs => 2,
			maxargs => 2,
			proc => \&listdownload
		},
		# v0.2
		"search.send" =>
		{
			desc => "Start hub search. Parameters: s/'search string'",
			minargs => 1,
			maxargs => 1,
			proc => \&searchsend
		},
		# v0.2
		"search.getresults" =>
		{
			desc => "Get search results. Parameters: none",
			proc => \&searchgetresults
		},
		# v0.2
		"show.version" =>
		{
			desc => "Show daemon version. Parameters: none",
			proc => \&showversion
		},
		# v0.2
		"show.ratio" =>
		{
			desc => "Show client ration. Parameters: none",
			proc => \&showratio
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

sub hubgetchat($$)
{
	my $req=RPC::XML::request->new('hub.getchat', RPC::XML::string->new($_[0]), RPC::XML::string->new($_[1] || $config{separator}));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub daemonstop()
{
	my $req=RPC::XML::request->new('daemon.stop');
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
	my $req=RPC::XML::request->new('share.list', RPC::XML::string->new($_[0] || $config{separator}));
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub sharerefresh()
{
	my $req=RPC::XML::request->new('share.refresh');
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

sub searchgetresults()
{
	my $req=RPC::XML::request->new('search.getresults');
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	foreach (@$res)
	{
		foreach ($_)
		{
			my %result=%$_;
			foreach (keys %result)
			{
				print("$_ => ${$result{$_}}\n");
			}
		}

	}
}

sub showversion()
{
	my $req=RPC::XML::request->new('show.version');
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}

sub showratio()
{
	my $req=RPC::XML::request->new('show.ratio');
	my $resp = $cli->send_request($req) or die "Can not send request to hub url $config{eiskaltURL}: $_\n";
	my $res = $P->parse($resp->as_string());
	print $$res."\n";
}



__END__


=pod
    xmlrpcRegistry.addMethod("magnet.add", magnetAddMethodP);
    xmlrpcRegistry.addMethod("daemon.stop", stopDaemonMethodP);
    xmlrpcRegistry.addMethod("hub.add", hubAddMethodP);
    xmlrpcRegistry.addMethod("hub.del", hubDelMethodP);
    xmlrpcRegistry.addMethod("hub.say", hubSayMethodP);
    xmlrpcRegistry.addMethod("hub.pm", hubSayPrivateMethodP);
    xmlrpcRegistry.addMethod("hub.list", listHubsMethodP);
    xmlrpcRegistry.addMethod("hub.getchat", getChatPubMethodP);
    xmlrpcRegistry.addMethod("share.add", addDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.rename", renameDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.del", delDirFromShareMethodP);
    xmlrpcRegistry.addMethod("share.list", listShareMethodP);
    xmlrpcRegistry.addMethod("share.refresh", refreshShareMethodP);
    xmlrpcRegistry.addMethod("list.download", getFileListMethodP);
    xmlrpcRegistry.addMethod("search.send", sendSearchMethodP);
    xmlrpcRegistry.addMethod("search.getresults", returnSearchResultsMethodP);
    xmlrpcRegistry.addMethod("show.version", showVersionMethodP);
    xmlrpcRegistry.addMethod("show.ratio", showRatioMethodP);
=cut

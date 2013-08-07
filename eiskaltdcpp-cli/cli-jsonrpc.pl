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
#
use strict;
#use diagnostics;
use warnings;
no warnings 'uninitialized';
use 5.012;
use JSON::RPC;
use Term::ShellUI;
use Data::Dumper;
use Getopt::Long;
use Env qw[$XDG_CONFIG_HOME $HOME];

# use non-standart paths
BEGIN {
    unshift @INC, 
	 "$XDG_CONFIG_HOME/eiskaltdc++",
	 "$HOME/.config/eiskaltdc++/",
	 "/usr/local/share/eiskaltdcpp/cli",
	 "/usr/share/eiskaltdcpp/cli"
}

# preparing terminal
use utf8;
use locale;
binmode STDOUT, ':utf8';

# configuration
our %config;
$config{version}=0.4;
$config{revision}=20130518;
require "cli-jsonrpc-config.pl";

# processing command line options
my $version,my $help, my $command;
GetOptions ('V|version' => \$version, 'h|help' => \$help, 'c=s' => \$command) or die("Invalid options");
if ($version)
{
	print("Command line JSON-RPC interface version.revision: $config{version}.$config{revision}\n");
	exit(1);
}
if ($help)
{
	print(  "Using:
\teiskaltdcpp-jcli
\teiskaltdcpp-jcli <Key>
This is command line JSON-RPC interface for eiskaltdcpp-daemon written on perl.
EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.

Keys:
\t-h, --help\t Show this message
\t-V, --version\t Show version string
\t-c command\t Execute jsonrpc method and exit\n");
	exit(1);
}
if ($config{debug} > 0)
{
	print("Configuration:\n");
	foreach (keys %config)
	{
		print("$_: $config{$_}\n");
	}
}

# rest variables
my $obj;
$obj->{'jsonrpc'} = $config{jsonrpc};
my $res;

# creating and configuring jsonrpc client
my $client;
if ( $JSON::RPC::VERSION >=  1.03 )
{
	require JSON::RPC::Legacy::Client;
	$client = new JSON::RPC::Legacy::Client;
}
else
{
	require JSON::RPC::Client;
	$client = new JSON::RPC::Client;
}
$client->version("2.0");
$client->ua->timeout(10);
#$client->ua->credentials('http://127.0.0.1:3121', 'jsonrpc', 'user' => 'password');
#$client->ua()->ssl_opts(verify_hostname=>0);

# preparing shell: reading command list, configuring history file and so
my $term = new Term::ShellUI(commands => get_commands(), history_file => $config{hist_file}, history_max => $config{hist_max});

# execute command if -c option used end exit 
if (defined($command))
{
	if ($config{debug} > 0) { print("Request: $command\n") };
	$term->process_a_cmd($command);
	exit(0);
}

# starting a shell if no -c option used
$term->prompt("$config{prompt}");
$term->run();

sub get_commands
{
	return
	{
		"magnet.add" =>
		{
			desc => "Add a magnet to download queue, and fetch it to download directory. Parameters: magnet, download directory",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&magnetadd
		},
		"daemon.stop" =>
		{
			desc => "Disconnect from hubs and exit, no params",
			proc => \&daemonstop
		},
		"hub.add" =>
		{
			desc => "Add a new hub and connect. Parameters: huburl, encoding",
			minargs => 2,
			maxargs => 2,
			proc => \&hubadd
		},
		"hub.del" =>
		{
			desc => "Disconnect hub and delete from autoconnect. Parameters: huburl",
			# add autocomplete from list of connected hubs
			minargs => 1,
			maxargs => 1,
			proc => \&hubdel
		},
		"hub.say" =>
		{
			desc => "Send a public message to hub. Parameters: huburl, message",
			# add autocomplete from list of connected hubs 
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&hubsay
		},
		"hub.pm" =>
		{
			desc => "Send a private message to nick on hub. Parameters: huburl, nick, message",
			# add autocomplete from connected hubs and nicks
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			minargs => 3,
			maxargs => 3,
			proc => \&hubpm
		},
		"hub.list" =>
		{
			desc => "Get a list of hubs, configured to connect to. Parameters: separator",
			#args => sub { grep { !/^\.?\.$/ } shift->complete_onlydirs(@_) },
			#minargs => 1,
			maxargs => 1,
			proc => \&hublist
		},
		"hub.listfulldesc" =>
		{
			desc => "Show detailed list of connected hubs. Parameters: none",
			proc => \&hublistfulldesc
		}, 
		"hub.getusers" =>
		{
			desc => "Show list of connected users for given hub. Parameters: huburl",
			minargs => 1,
			maxargs => 1,
			proc => \&hubgetusers
		},
		"hub.getuserinfo" =>
		{
			desc => "Show detailed information for given user nick and hub. Parameters: nick, huburl",
			minargs => 2,
			maxargs => 2,
			proc => \&hubgetuserinfo
		},
		"share.add" =>
		{
			desc => "Add a directory to share as virtual name. Parameters: directory, virtual name",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&shareadd
		},
		"share.rename" =>
		{
			desc => "Give a new virtual name to shared directory. Parameters: directory, virtual name",
			args => sub { shift->complete_onlydirs(@_) },
			minargs => 2,
			maxargs => 2,
			proc => \&sharerename
		},
		"share.del" =>
		{
			desc => "Unshare directory with virtual name. Parameters: Parameters: virtual name",
			minargs => 1,
			maxargs => 1,
			proc => \&sharedel
		},
		"share.list" =>
		{
			desc => "Get a list of shared directories. Parameters: separator",
			maxargs => 1,
			proc => \&sharelist
		},

		"share.refresh" =>
		{
			desc => "Refresh a share, hash up new files. Parameters: none",
			proc => \&sharerefresh
		},
		"list.download" =>
		{
			desc => "Download a file list from nick on huburl. Parameters: huburl, nick",
			minargs => 2,
			maxargs => 2,
			proc => \&listdownload
		},
		"hub.getchat" =>
		{
			desc => "Get last public chat messages. Parameters: huburl, separator",
			minargs => 1,
			maxargs => 2,
			proc => \&hubgetchat
		},
		"search.send" =>
		{
			desc => "Start hub search. Parameters: search string",
			minargs => 1,
			maxargs => 1,
			proc => \&searchsend
		},
		"search.getresults" =>
		{
			desc => "Get search results from all hubs (or only from specified). Parameters: huburl",
			maxargs => 1,
			proc => \&searchgetresults
		},
		"show.version" =>
		{
			desc => "Show daemon version. Parameters: none",
			proc => \&showversion
		},
		"show.ratio" =>
		{
			desc => "Show client ratio. Parameters: none",
			proc => \&showratio
		},
		"queue.setpriority" =>
		{
			desc => "Set download queue priority. Parameters: target, proirity. Priority is an integer from 0(paused) to 3(high)",
			minargs => 2,
			maxargs => 2,
			proc => \&qsetprio
		},
		"queue.move" =>
		{
			desc => "Move queue item from source to target. Parameters: source, target",
			minargs => 2,
			maxargs => 2,
			proc => \&qmove
		},
		"queue.remove" =>
		{
			desc => "Delete queue item by target. Parameters: target",
			minargs => 1,
			maxargs => 1,
			proc => \&qremove
		},
		"queue.list" =>
		{
			desc => "Show queue, including all targets. Parameters: none",
			proc => \&qlist
		},
		"queue.listtargets" =>
		{
			desc => "Show  all targets. Parameters: none",
			proc => \&qlisttargets
		},
		"search.clear" =>
		{
			desc => "Clear search results. Parameters: huburl",
			maxargs => 1,
			proc => \&searchclear
		},
		"queue.getsources" =>
		{
			desc => "Show sources for specified target. Parameters: target, separator",
			minargs => 1,
			maxargs => 2,
			proc => \&qgetsources
		},
		"hash.status" =>
		{
			desc => "Show hashing process status. Parameters: none",
			proc => \&hashstatus
		},
		"hash.pause" =>
		{
			desc => "Pause hashing process. Parameters: none",
			proc => \&hashpause
		},
		"methods.list" =>
		{
			desc => "List all jsonrpc methods available. Parameters: none",
			proc => \&methodslist
		},
		"queue.matchlists" =>
		{
			desc => "Description here. Parameters: none",
			proc => \&qmatchlists
		},
		# last
		"prompt" =>
		{
			desc => "Set custom prompt",
			minargs => 1,
			maxargs =>1,
			proc => sub { $term->prompt(shift) }
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
		},
		"help" => 
		{ 
			desc => "Print helpful information",
			args => sub { shift->help_args(undef, @_); },
			method => sub { shift->help_call(undef, @_); } 
		}
	}
}

sub magnetadd($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'magnet.add';
	$obj->{'params'}->{'magnet'}=$_[0];
	$obj->{'params'}->{'directory'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub daemonstop()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'daemon.stop';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub hubadd($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.add';
	$obj->{'params'}->{'huburl'}=$_[0];
	$obj->{'params'}->{'enc'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hubdel($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.del';
	$obj->{'params'}->{'huburl'}=$_[0];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hubsay($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.say';
	$obj->{'params'}->{'huburl'}=$_[0];
	$obj->{'params'}->{'message'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hubpm($$$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.pm';
	$obj->{'params'}->{'huburl'}=$_[0];
	$obj->{'params'}->{'nick'}=$_[1];
	$obj->{'params'}->{'message'}=$_[2];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hublist($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.list';
	$obj->{'params'}->{'separator'}=($_[0] || $config{'separator'});
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hublistfulldesc($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.listfulldesc';
	$obj->{'params'}->{'separator'}=($_[0] || $config{'separator'});
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			if (defined($res->result))
			{
				for my $key (keys %{$res->result})
				{
					print("$key:\n");
					foreach ($res->result->{$key})
					{
						print("Total share:\t\t".$_->{totalshare}."\n");
						print("Connected:\t\t".$_->{connected}."\n");
						print("Users:\t\t".$_->{users}."\n");
						print("Description:\t\t".$_->{description}."\n");
						print("Hub name:\t\t".$_->{hubname}."\n");
						
					}
				}
			}
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}


sub hubgetusers()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.getusers';
	$obj->{'params'}->{'huburl'}=$_[0];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			print(join("\n",split(/;/,$res->result))."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hubgetuserinfo()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.getuserinfo';
	$obj->{'params'}->{'nick'}=$_[0];
	$obj->{'params'}->{'huburl'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			if (defined($res->result))
			{
				#for my $key (keys %{$res->result})
				#{
				#	print("$key:\t\t$res->result->{$key}\n");
				#}
				while ( my ($key, $value) = each(%{$res->result}) )
				{
					print "$key:\t\t$value\n";
				}
			}
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub shareadd($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'share.add';
	$obj->{'params'}->{'directory'}=$_[0];
	$obj->{'params'}->{'virtname'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub sharerename($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'share.rename';
	$obj->{'params'}->{'directory'}=$_[0];
	$obj->{'params'}->{'virtname'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub sharedel($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'share.del';
	$obj->{'params'}->{'directory'}=$_[0];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub sharelist($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'share.list';
	$obj->{'params'}->{'separator'}=($_[0] || $config{'separator'});
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub sharerefresh()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'share.refresh';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub listdownload($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'list.download';
	$obj->{'params'}->{'huburl'}=$_[0];
	$obj->{'params'}->{'nick'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hubgetchat($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hub.getchat';
	$obj->{'params'}->{'huburl'}=$_[0];
	$obj->{'params'}->{'separator'}=($_[1] || $config{'separator'});
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub searchsend($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'search.send';
	$obj->{'params'}->{'searchstring'}=$_[0];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub searchgetresults($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'search.getresults';
	if (defined($_[0])) {$obj->{'params'}->{'huburl'}=$_[0]};
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			foreach (@{$res->result})
			{
				print("CID:\t\t".$_->{CID}."\n");
				print("Connection:\t\t".$_->{Connection}."\n");
				print("Exact Size:\t\t".$_->{'Exact Size'}."\n");
				print("File Order:\t\t".$_->{'File Order'}."\n");
				print("Filename:\t\t".$_->{Filename}."\n");
				print("Free Slots:\t\t".$_->{'Free Slots'}."\n");
				print("Hub:\t\t".$_->{Hub}."\n");
				print("Hub URL:\t\t".$_->{'Hub URL'}."\n");
				print("Icon:\t\t".$_->{Icon}."\n");
				print("IP:\t\t".$_->{IP}."\n");
				print("Nick:\t\t".$_->{Nick}."\n");
				print("Path:\t\t".$_->{Path}."\n");
				print("Real Size:\t\t".$_->{'Real Size'}."\n");
				print("Shared:\t\t".$_->{Shared}."\n");
				print("Size:\t\t".$_->{Size}."\n");
				print("Slots:\t\t".$_->{Slots}."\n");
				print("Slots Order:\t\t".$_->{'Slots Order'}."\n");
				print("TTH:\t\t".$_->{TTH}."\n");
				print("Type:\t\t".$_->{Type}."\n\n");
			}
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub searchclear($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'search.clear';
	if (defined($_[0])) {$obj->{'params'}->{'huburl'}=$_[0]};
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub showversion()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'show.version';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub showratio()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'show.ratio';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			print("Ratio:\t\t".$res->result->{ratio}."\n");
			print("Upload:\t\t".$res->result->{up}."\n");
			print("Download:\t".$res->result->{down}."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub qsetprio($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.setpriority';
	$obj->{'params'}->{'target'}=$_[0];
	$obj->{'params'}->{'priority'}=$_[1]+0;
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub qmove($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.move';
	$obj->{'params'}->{'source'}=$_[0];
	$obj->{'params'}->{'target'}=$_[1];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub qremove($)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.remove';
	$obj->{'params'}->{'target'}=$_[0];
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub qlist()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.list';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			if (defined($res->result))
			{
				for my $key (keys %{$res->result})
				{
					print("$key:\n");
					foreach ($res->result->{$key})
					{
						print("Added:\t\t".$_->{Added}."\n");
						print("Downloaded:\t\t".$_->{Downloaded}."\n");
						print("Downloaded Sort:\t\t".$_->{'Downloaded Sort'}."\n");
						print("Errors:\t\t".$_->{Errors}."\n");
						print("Exact Size:\t\t".$_->{'Exact Size'}."\n");
						print("Filename:\t\t".$_->{Filename}."\n");
						print("Path:\t\t".$_->{Path}."\n");
						print("Priority:\t\t".$_->{Priority}."\n");
						print("Size:\t\t".$_->{Size}."\n");
						print("Size Sort:\t\t".$_->{'Size Sort'}."\n");
						print("Status:\t\t".$_->{Status}."\n");
						print("Target:\t\t".$_->{Target}."\n");
						print("TTH:\t\t".$_->{TTH}."\n");
						print("Users:\t\t".$_->{Users}."\n\n");
					}
				}
			}
			else {print("No files in queue\n");}
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub qlisttargets()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.listtargets';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub qgetsources($$)
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.getsources';
	$obj->{'params'}->{'target'}=$_[0];
	$obj->{'params'}->{'separator'}=($_[1] || $config{'separator'});
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			print("Online:\t\t".$res->result->{online}."\n");
			print("Sources:\t\t".$res->result->{sources}."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
	delete($obj->{'params'});
}

sub hashstatus()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hash.status';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n");
			print("Bytesleft:\t\t".$res->result->{bytesleft}."\n");
			print("Currentfile:\t\t".$res->result->{currentfile}."\n");
			print("Filesleft:\t\t".$res->result->{filesleft}."\n");
			print("Status:\t\t".$res->result->{status}."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub hashpause()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'hash.pause';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub methodslist()
{
	use JSON;
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'methods.list';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n")};
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}

sub qmatchlists()
{
	$obj->{'id'} = int(rand(2**16));
	$obj->{'method'} = 'queue.matchlists';
	if ($config{debug} > 0) { print("===Request===\n".Dumper($obj)."\n") };
	$res = $client->call($config{eiskaltURL}, $obj);
	if ($res)
	{
		if ($res->is_error) 
		{
			print("===Error===\nCode: ".$res->error_message->{'code'}."===\n".$res->error_message->{'message'}."\n");
		}
		else
		{
			print("===Reply===\n".$res->result."\n");
		}
	}
	else
	{
		print $client->status_line;
	}
}


__END__

=pod

# known methods as for 0.2.17062012
# grep "AddMethod" eiskaltdcpp-daemon/ServerThread.cpp | egrep -o "std::string\(.*\)"
magnet.add +0.3
daemon.stop +0.3
hub.add +0.3
hub.del +0.3
hub.say +0.3
hub.pm +0.3
hub.list +0.3
hub.getchat +0.3
share.add +0.3
share.rename +0.3
share.del +0.3
share.list +0.3
share.refresh +0.3
list.download +0.3
search.send +0.3
search.getresults +0.3
search.clear +0.3
show.version +0.3
show.ratio +0.3
queue.setpriority -
queue.move -
queue.remove +0.3
queue.listtargets +0.3
queue.list +0.3
queue.getsources +0.3
hash.status +0.3
hash.pause +0.3
methods.list +0.3
queue.matchlists +0.3
hub.listfulldesc +0.4
hub.getusers
hub.getuserinfo

=cut


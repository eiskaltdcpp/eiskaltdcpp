/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sf.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

namespace dht
{

#define BOOTSTRAP_URL				"http://strongdc.sourceforge.net/bootstrap/"

#define DHT_UDPPORT					6250							// default DHT port
#define DHT_FILE					"dht.xml"						// local file with all information got from the network

#define ID_BITS						192								// size of identificator (in bits)

#define CONNECTED_TIMEOUT			20*60*1000	// 20 minutes		// when there hasn't been any incoming packet for this time, network will be set offline

#define ADC_PACKET_HEADER			'U'								// byte which every uncompressed packet must begin with
#define ADC_PACKET_FOOTER			0x0a							// byte which every uncompressed packet must end with
#define ADC_PACKED_PACKET_HEADER	0xc1							// compressed packet detection byte

#define SEARCH_ALPHA				3								// degree of search parallelism
#define MAX_SEARCH_RESULTS			300								// maximum of allowed search results
#define SEARCH_PROCESSTIME			3*1000	// 3 seconds			// how often to process done search requests				
#define SEARCH_STOPTIME				15*1000	// 15 seconds			// how long to wait for delayed search results before deleting the search
#define SEARCHNODE_LIFETIME			20*1000	// 45 seconds			// how long to try searching for node
#define SEARCHFILE_LIFETIME			45*1000	// 45 seconds			// how long to try searching for file
#define SEARCHSTOREFILE_LIFETIME	20*1000	// 20 seconds			// how long to try publishing a file
#define SELF_LOOKUP_TIMER			4*60*60*1000	// 4 hours		// how often to search for self node

#define K							10								// maximum nodes in one bucket

#define MAX_PUBLISHED_FILES			200								// max local files to publish
#define MIN_PUBLISH_FILESIZE		1024 * 1024 // 1 MiB			// files below this size won't be published
#define REPUBLISH_TIME				5*60*60*1000	// 5 hours		// when our filelist should be republished
#define PFS_REPUBLISH_TIME			1*60*60*1000	// 1 hour		// when partially downloaded files should be republished
#define MAX_PUBLISHES_AT_TIME		3								// how many files can be published at one time
#define PUBLISH_TIME				2*1000	// 2 seconds			// how often publishes files

#define FW_RESPONSES				3								// how many UDP port checks are needed to detect we are firewalled
#define FWCHECK_TIME				1*60*60*1000					// how often request firewalled UDP check

#define NODE_RESPONSE_TIMEOUT		2*60*1000	// 2 minutes		// node has this time to response else we ignore him/mark him as dead node
#define NODE_EXPIRATION				2*60*60*1000 // 2 hours			// when node should be marked as possibly dead

#define TIME_FOR_RESPONSE			3*60*1000	// 3 minutes		// node has this time to respond to request; after that response will be marked as unwanted

#define CLIENT_PROTOCOL				"ADC/1.0"						// protocol used for file transfers
#define SECURE_CLIENT_PROTOCOL_TEST	"ADCS/0.10"						// protocol used for secure file transfers
#define ADCS_FEATURE				"ADC0"							// support for secure protocol
#define TCP4_FEATURE				"TCP4"							// support for active TCP
#define UDP4_FEATURE				"UDP4"							// support for active UDP
#define DHT_FEATURE					"DHT0"

const std::string NetworkName =		"DHT";

using namespace dcpp;

}

#endif	// _CONSTANTS_H
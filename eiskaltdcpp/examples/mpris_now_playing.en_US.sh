#!/bin/bash
#
# This scrip is disigned to show what is playing now in a mpris compatible media player
#
# Author: Vovochka404 (vovochka13 (at) gmail.com)
# License: Public Domain
# Depends: bash, qdbus
#
# Example output:
# /me is listening to Three Days Grace - Wake Up (Three Days Grace) via Amarok
#
# Player support:
# For qmmp: preference -> modules -> general -> module MPRIS
# For vlc: preference (full view) -> interface -> interface control -> use dbus
# For audacious: Native
# For amarok: Native
# For dragon (KDE Dragon Player): Native
# For mpd (Music Player Daemon): http://mpd.wikia.com/wiki/Client:MpDris
# For songbird: http://addons.songbirdnest.com/addon/1626
# For clementine: Native since 0.3
# For exaile: Plugin. See http://www.exaile.org/wiki/KDE_and_MPRIS_plugin
#
# See also:
# http://xmms2.org/wiki/MPRIS
# http://incise.org/mpris-remote.html


PLAYERS="amarok audacious qmmp mpd xmms2 dragon vlc songbird clementine exaile";

CONFIG_FILE="$HOME/.eiskaltdc++/mpris_now_playing.conf";

METADATA_CALL="/Player org.freedesktop.MediaPlayer.GetMetadata"
PLAYER='';

# Trying to detect mpris compatible player and get info.

for P in $PLAYERS; do
	DBUS=`qdbus | grep "org.mpris.$P" | awk '{print $1}'`;
	if [[ $DBUS && $(qdbus $DBUS $METADATA_CALL 2>/dev/null) ]]
	then
		METAINFO=$(qdbus $DBUS $METADATA_CALL 2>/dev/null);
		PLAYER=$P;
		TITLE=$(echo "$METAINFO" | sed -e '/^title: / !d' -e s/'title: '//);
		ARTIST=$(echo "$METAINFO" | sed -e '/^artist: / !d' -e s/'artist: '//);
		ALBUM=$(echo "$METAINFO" | sed -e '/^album: / !d' -e s/'album: '//);
		GENRE=$(echo "$METAINFO" | sed -e '/^genre: / !d' -e s/'genre: '//);
		LOCATION=$(echo "$METAINFO" | sed -e '/^location: file:\/\// !d' -e s/'location: file:\/\/'//); # Works only for local files.
		break;
	fi
done

# Some beautiful names for players

case $PLAYER in
	amarok)
		PLAYER="Amarok";
		;;
	audacious)
		PLAYER="Audacious2";
		;;
	dragon)
		PLAYER="Dragon Player";
		;;
	vlc)
		PLAYER="VLC";
		;;
	clementine)
		PLAYER="Clementine";
		;;
esac

# Trying to load home config

if [ ! -e $CONFIG_FILE ]
then
# Got no config file. Let's write basic config.
	echo "#" >> $CONFIG_FILE;
	echo "# This is an example config for mpris_now_playing scrip" >> $CONFIG_FILE;
	echo "#" >> $CONFIG_FILE;
	echo "if [ \$PLAYER ]" >> $CONFIG_FILE;
	echo "then" >> $CONFIG_FILE;
	echo "	NOW_LISTENING_TO=\"/me is listening to \$ARTIST - \$TITLE (\$ALBUM) via \$PLAYER <magnet>\$LOCATION</magnet>\"" >> $CONFIG_FILE;
	echo "else" >> $CONFIG_FILE;
	echo "	NOW_LISTENING_TO=\"/me is listening to mouse clicks\"" >> $CONFIG_FILE;
	echo "fi" >> $CONFIG_FILE;
fi

. $CONFIG_FILE;

# Let's test what we'he got

if [ "$NOW_LISTENING_TO" ]
then
	echo "$NOW_LISTENING_TO";
else
	echo "/me is fool.";
fi


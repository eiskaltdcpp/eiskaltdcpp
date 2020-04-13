#!/bin/bash
#
# This script is designed to show what is playing now in a mpris compatible media player
#
# Authors: Vovochka404 (vovochka13 (at) gmail.com), Nikoli <nikoli[at]lavabit.com>
# License: Public Domain
# Depends: bash, qdbus
#
# Example output:
# /me is listening to Three Days Grace - Wake Up (Three Days Grace) via Amarok
#
# Player support:
# For qmmp: Settings -> Plugins -> General -> MPRIS plugin
# For vlc: Preferences (full view) -> Interface -> Control interface -> D-bus control interface
# For audacious: Native
# For amarok: Native
# For dragon (KDE Dragon Player): Native
# For mpd (Music Player Daemon): http://mpd.wikia.com/wiki/Client:MpDris
# For songbird: http://addons.songbirdnest.com/addon/1626
# For clementine: Native since 0.3
# For exaile: Plugin. See http://www.exaile.org/wiki/KDE_and_MPRIS_plugin
# For deadbeef: Plugin: https://github.com/Serranya/deadbeef-mpris2-plugin
#
# See also:
# https://github.com/xmms2/wiki/wiki/MPRIS
# http://incise.org/mpris-remote.html


CONFIG_FILE="${HOME}/.config/eiskaltdc++/mpris_now_playing2.conf"

METADATA_CALL="/Player org.freedesktop.MediaPlayer.GetMetadata"
METADATA_CALL_V2="/org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Metadata"

STATUS_CALL="/Player org.freedesktop.MediaPlayer.GetStatus"
STATUS_CALL_V2="/org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.PlaybackStatus"

PLAYER=''
declare -A tag
declare -A paused

QDBUS=$(which qdbus 2>/dev/null);
if [ -z "$QDBUS" ]; then
	QDBUS=$(which qdbus-qt5 2>/dev/null);
fi
if [ -z "$QDBUS" ]; then
	echo "/me is missing qdbus or qdbus-qt5... :(";
	exit 0;
fi


DBUS_COLLECTION=$(${QDBUS} | grep "org.mpris.");

for app in $DBUS_COLLECTION; do
	IS_MPRIS2=$(echo $app | grep "MediaPlayer2" | grep -v "grep" | wc -l);
	PLAYER=$(echo $app | sed -r "s/.+\.//g" );
	if [ $IS_MPRIS2 -eq 0 ]; then
		MCALL=$METADATA_CALL;
		SCALL=$STATUS_CALL;
		_AP="--literal";
	else
		MCALL=$METADATA_CALL_V2;
		SCALL=$STATUS_CALL_V2;
		_AP="";
	fi
	METAINFO="$(${QDBUS} "${app}" ${MCALL} 2>/dev/null)";
	if [ -n "$METAINFO" ]; then
		STATUS="$($QDBUS ${_AP} "${app}" ${SCALL} 2>/dev/null)";
		if [ $IS_MPRIS2 -eq 0 ]; then
			STATUS=$(echo $STATUS | sed -r -e "s/\\[Argument: \\(iiii\\) ([0-9]+), .+\\]/\\1/");
			if [ "$STATUS" == "0" ]; then
				STATUS="Playing";
			else
				STATUS="Paused";
			fi
		fi
		for i in album artist genre title; do
			tag[${i}]="$(echo "${METAINFO}" | sed -r -e "/^(xesam:)?${i}: / !d" -e "s/^(xesam:)?${i}: //")"
		done
		# Works only for local files:
		tag["location"]="$(echo "${METAINFO}" | sed -r -e '/^(location|xesam:url): file:\/\// !d' -e 's/(location|xesam:url): file:\/\///' | sed -n -e's/%\([0-9A-F][0-9A-F]\)/\\x\1/g' -e's/+/ /g' -e's/.*/echo -e "&"/g' -ep | "${SHELL}")"
		tag["player"]=$PLAYER;
		if [ "$STATUS" == "Playing" ]; then
			break;
		else
			for i in album artist genre title location player; do
			paused[$i]=${tag[$i]};
			done
		fi
	else
		PLAYER="";
		STATUS="";
		tag["player"]="";
	fi
done

# If last player was not playing - getting last paused player.
if [ "$STATUS" != "Playing" ] && [ -n "${paused[player]}" ]; then
	for i in album artist genre title location player; do
		tag[$i]=${paused[$i]};
	done
fi;


# Some beautiful names for players
case ${tag["player"]} in
    amarok)
        tag["player"]="Amarok"
        ;;
    audacious)
        tag["player"]="Audacious2"
        ;;
    dragon)
        tag["player"]="Dragon Player"
        ;;
    vlc)
        tag["player"]="VLC"
        ;;
    clementine)
        tag["player"]="Clementine"
        ;;
esac

# Trying to load home config

# If got no config file, let's write basic config.
[ ! -e "${CONFIG_FILE}" ] && cat >> "${CONFIG_FILE}" << _EOF_
#
# This is an example config for mpris_now_playing script
#
if [ -n "\${tag[player]}" ];
then
    NOW_LISTENING_TO="/me is listening to \${tag[artist]} - \${tag[title]} (\${tag[album]}) via \${tag[player]} <magnet>\${tag[location]}</magnet>"
else
    NOW_LISTENING_TO="/me is listening to mouse clicks"
fi
_EOF_

. "${CONFIG_FILE}"

# Let's test what we'he got
if [ "${NOW_LISTENING_TO}" ]
then
    echo "${NOW_LISTENING_TO}"
else
    echo "/me is..."
fi

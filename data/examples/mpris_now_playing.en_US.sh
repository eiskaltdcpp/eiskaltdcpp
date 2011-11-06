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
#
# See also:
# http://xmms2.org/wiki/MPRIS
# http://incise.org/mpris-remote.html


PLAYERS="amarok audacious qmmp mpd xmms2 dragon vlc songbird clementine exaile"

CONFIG_FILE="${HOME}/.config/eiskaltdc++/mpris_now_playing.conf"

METADATA_CALL="/Player org.freedesktop.MediaPlayer.GetMetadata"
PLAYER=''
declare -A tag

# Trying to detect mpris compatible player and get info.
for P in ${PLAYERS}; do
    DBUS="$(qdbus | grep "org.mpris.${P}" | awk '{print $1}')"
    if [[ "${DBUS}" && $(qdbus "${DBUS}" ${METADATA_CALL} 2>/dev/null) ]]
    then
        METAINFO="$(qdbus "${DBUS}" ${METADATA_CALL} 2>/dev/null)"
        PLAYER="${P}"
        for i in album artist genre title; do
            tag[${i}]="$(echo "${METAINFO}" | sed -e "/^${i}: / !d" -e "s/^${i}: //")"
        done
        # Works only for local files:
        LOCATION="$(echo "${METAINFO}" | sed -e '/^location: file:\/\// !d' -e 's/location: file:\/\///' | sed -n -e's/%\([0-9A-F][0-9A-F]\)/\\x\1/g' -e's/+/ /g' -e's/.*/echo -e "&"/g' -ep | "${SHELL}")"
    fi
done

# Some beautiful names for players
case ${PLAYER} in
    amarok)
        PLAYER="Amarok"
        ;;
    audacious)
        PLAYER="Audacious2"
        ;;
    dragon)
        PLAYER="Dragon Player"
        ;;
    vlc)
        PLAYER="VLC"
        ;;
    clementine)
        PLAYER="Clementine"
        ;;
esac

# Trying to load home config

# If got no config file, let's write basic config.
[ ! -e "${CONFIG_FILE}" ] && cat >> "${CONFIG_FILE}" << _EOF_
#
# This is an example config for mpris_now_playing script
#
if [ "\${PLAYER}" ]
then
    NOW_LISTENING_TO="/me is listening to \${tag[artist]} - \${tag[title]} (\${tag[album]}) via \${PLAYER} <magnet>\${LOCATION}</magnet>"
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

#!/bin/sh

# This script shows that are now played in qmmp

## Author: none
## License: Public Domain

## Depends: sh, qmmp
## Notes: MPRIS plugin must be enabled in qmmp settings

## Examples:
# output in console:
# /me Audacious playing now: Rammstein - Sonne
# /me слушает Orjan Nilsen - So Long Radio
# output in eiskaltdcpp chat:
# [13:53:44] *  ** visual слушает Orjan Nilsen - So Long Radio

export DISPLAY=:0
MESSAGE=""
NOWPLAYING=`qdbus org.mpris.qmmp /Player org.freedesktop.MediaPlayer.GetMetadata`
if [ $? = 0 ] && [ -n "$NOWPLAYING" ]; then
ARTIST=`echo "$NOWPLAYING" | sed -ne 's/^artist: \(.*\)$/\1/p'`
TRACK=`echo "$NOWPLAYING" | sed -ne 's/^title: \(.*\)$/\1/p'`
MESSAGE="слушает $ARTIST - $TRACK"
fi

echo -e "/me $MESSAGE"


#!/bin/sh

# This script tells about song played in amarok2

## Author: WiseLord
## License: Public Domain

## Depends: bash, amarok2

## Examples:
# output in console:
# /me is listening now: Therion - Midgård
# output in eiskaltdcpp chat:
# [17:45:03] * WiseLord is listening now: Therion - Midgård

if [ -n "$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata)" ]
then
	title=$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata | sed -e '/^title/ !d' -e s/'title: '//)
	artist=$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata | sed -e '/^artist/ !d' -e s/'artist: '//)
	echo /me is listening now: $artist - $title
else
	echo /me is listening silence now
fi

#!/bin/sh

# This script tells about song played in amarok2

## Author: WiseLord
## License: Public Domain
## Version: 0.2

## Depends: sh, amarok2

## Examples:
# output in console:
# /me is listening now: Therion - Midgård
# output in eiskaltdcpp chat:
# [17:45:03] * WiseLord is listening now: Therion - Midgård

nowPlaying=$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata 2>/dev/null)
if [ -n "$nowPlaying" ]
then
	title=$(echo "$nowPlaying" | sed -ne 's/^title: \(.*\)$/\1/p')
	artist=$(echo "$nowPlaying" | sed -ne 's/^artist: \(.*\)$/\1/p')
	echo /me is listening now: $artist - $title
else
	echo /me is listening silence now
fi

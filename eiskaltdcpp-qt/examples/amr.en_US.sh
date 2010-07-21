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

message="/me is listening silence now"
nowPlaying="$(qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata 2>/dev/null)"

if [ -n "${nowPlaying}" ]
then
	title="$(echo "${nowPlaying}" | sed -ne 's/^title: \(.*\)$/\1/p')"
	artist="$(echo "${nowPlaying}" | sed -ne 's/^artist: \(.*\)$/\1/p')"
	message="/me is listening now: ${artist} - ${title}"

## Also you can send song magnet to chat:
# [17:45:03] * WiseLord is listening now: Therion - Midgård (02. Midgård.mp3) (7.0 МиБ)
# You can use <magnet show=NAME_TO_SHOW>PATH_TO_FILE</magnet> or just <magnet>PATH_TO_FILE</magnet>
# If you want to do this, uncomment 2 lines below:

# 	location="$(echo "${nowPlaying}" | sed -ne 's/^location: file:\/\/\(.*\)$/\1/p' | sed -e s/\'/\\\\\'/g -e 's/%\([0-9A-Fa-f][0-9A-Fa-f]\)/\\\\\x\1/g' | xargs echo -e )"
# 	message="/me is listening now: ${artist} - ${title} ( <magnet>${location}</magnet> )"
fi

echo "${message}"

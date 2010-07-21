#!/bin/sh

# This script shows that are now played in mpd

## Author: none
## License: Public Domain

## Depends: sh, mpc, mpd

## Examples:
# output in console:
# /me listening now Blind Guardian - The Bard's Song: In The Forest
# output in eiskaltdcpp chat:
# [17:45:03] * dhampire listening now Blind Guardian - The Quest For Tanelorn

mpc="$(mpc --format "%artist% - %title%" | head -n 1)"
echo "/me listening now ${mpc}"

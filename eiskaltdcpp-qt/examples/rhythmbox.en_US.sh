#!/bin/sh

# This script shows that are now played in Rhythmbox

## Author: none
## License: Public Domain

## Depends: sh, rhythmbox

## Examples:
# output in console:
# /me Rhythmbox playing now: Rammstein - Sonne
# output in eiskaltdcpp chat:
# [10:57:01] * Ben_Vladen Rhythmbox playing now: Rammstein - Sonne

rh="$(rhythmbox-client --print-playing)"
echo "/me Rhythmbox playing now: ${rh}"

#!/bin/sh

# This script shows that are now played in Rhythmbox

## Author: none
## License: Public Domain

## Depends: sh, rhythmbox

## Examples:
# output in console:
#
# output in eiskaltdcpp chat:
#

rh=$(rhythmbox-client --print-playing)
echo "/me Rhythmbox playing now: $rh"


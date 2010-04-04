#!/bin/sh

# This script shows that are now played in Audacious

## Author: none
## License: Public Domain

## Depends: sh, audacious

## Examples:
# output in console:
#
# output in eiskaltdcpp chat:
#

au=$(audtool2 --current-song)
echo "/me Audacious playing now: $au"


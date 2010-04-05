#!/bin/sh

# This script shows that are now played in Audacious

## Author: none
## License: Public Domain

## Depends: sh, audacious

## Examples:
# output in console:
# /me Audacious playing now: Rammstein - Sonne
# output in eiskaltdcpp chat:
# [10:57:03] * Ben_Vladen Audacious playing now: Rammstein - Sonne

au=$(audtool2 --current-song)
echo "/me Audacious playing now: $au"


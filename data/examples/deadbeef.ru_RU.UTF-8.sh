#!/bin/bash
# This script is designed to show what is playing now in a DeaDBeeF music player
#
# Authors: Unreal7z (unreal777z (at) gmail.com)
# License: Public Domain
# Depends: bash
#
# Example output:
# console: /me DeaDBeeF playing now:Black Heaven - Himmel ohne Sterne, длина песни 3:59, уже прослушал 2:38 ( <magnet>/media/Music/Фашисты/Black Heaven/Black Heaven - Discography (2001-2011), AAC/2011 - Dystopia/07 Himmel ohne Sterne.m4a</magnet> ) [Powered by DeaDBeeF 0.5.1 | GNU/Linux]

# chat: /me DeaDBeeF playing now:Black Heaven - Himmel ohne Sterne, длина песни 3:59, уже прослушал 2:54 ( 07 Himmel ohne Sterne.m4a (9.1 МиБ) ) [Powered by DeaDBeeF 0.5.1 | GNU/Linux]]
# See also:
# http://sourceforge.net/apps/mediawiki/deadbeef/index.php?title=Title_Formatting

db=$(deadbeef --nowplaying "%a - %t, длина песни %l, уже прослушал %e")
directory=$(deadbeef --nowplaying "%D")
version=$(deadbeef --nowplaying "%V")
file=$(deadbeef --nowplaying "%f")
echo /me DeaDBeeF playing now:$db "( <magnet>$directory/$file</magnet> )" "[Powered by DeaDBeeF $version | GNU/Linux]"
exit

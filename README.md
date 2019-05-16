# EiskaltDC++ &ndash; file sharing using DC and ADC protocols

Website: [https://sourceforge.net/projects/eiskaltdcpp](https://sourceforge.net/projects/eiskaltdcpp/) <br>
Sources: [https://github.com/eiskaltdcpp/eiskaltdcpp](https://github.com/eiskaltdcpp/eiskaltdcpp)

Public chat room: https://gitter.im/eiskaltdcpp/eiskaltdcpp <br>
Wiki: https://github.com/eiskaltdcpp/eiskaltdcpp/wiki

## License

This program is licensed under the GNU General Public License. See the [COPYING](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/COPYING) file for more information.

## Description

EiskaltDC++ is a cross-platform program that uses the [Direct Connect](https://en.wikipedia.org/wiki/Direct_Connect_\(protocol\)) (DC aka NMDC) and [Advanced Direct Connect](https://en.wikipedia.org/wiki/Advanced_Direct_Connect) (ADC) protocols. It is compatible with DC++, FlylinkDC++, LinuxDC++ and other DC clients. EiskaltDC++ also interoperates with all common DC hub software.

Currently supported systems (in order of decreasing importance): GNU/Linux, macOS, MS Windows, FreeBSD, Haiku and GNU/Hurd.

Currently supported features (not full list):

* 1
* 2
* 3

<a href="https://tehnick.github.io/eiskaltdcpp/eiskaltdcpp-qt-2.2.10-588_search_widget.png" title="Example of search results">
    <img src="https://tehnick.github.io/eiskaltdcpp/eiskaltdcpp-qt-2.2.10-588_search_widget.png" width="99%">
</a>

## Versions history

See [ChangeLog.txt](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/ChangeLog.txt) file.

## Installation

For build from sources see [INSTALL](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/INSTALL) file.

GNU/Linux and FreeBSD users may install packages from official and unofficial repositories, ports, etc.

macOS users may install and update official builds using [Homebrew](https://brew.sh/) cask:

```
brew cask install eiskaltdcpp
```

or download app bundles from SourceForge (see links below) and install them manually. Program doesn't have embedded mechanism of updates, so in this case users should monitor updates themselves.


Haiku users may install official package from HaikuPorts:

```
pkgman install eiskaltdcpp
```

MS Windows users may download official installers and portable builds from SourceForge (see links below). Program doesn't have embedded mechanism of updates, so users should monitor updates themselves.

## Usage

EiskaltDC++ is designed mostly for experienced users who know how DC works which settings should be used with their type of network connection, etc.. This short introduction is not intended to describe such basic things.

Here are small recommendations for initial configuration after first launch of program:

1. Open Preferences dialog and set nickname (it is important), default encoding for DC hubs (for example, WINDOWS-1251 for Russian hubs), type of incoming connections (active mode, firewall with UPnP or passive mode) and downloads directory.
2. Open Public Hubs widget and connect to few popular public hubs (you may sort list by amount of users or by total amount of shared data).
3. Open Search widget and try to search any popular file. (Do not forget to use spaces as words separator.)
4. Add few hubs to list of Favourite hubs and enable auto connect on program startup for them.
5. Program settings will be saved on program exit.

All programs from EiskaltDC++ project (`eiskaltdcpp-qt`, `eiskaltdcpp-gtk` and `eiskaltdcpp-daemon`) use the same common settings. So once you have correctly configured connections in EiskaltDC++ Qt, for example, you may launch `eiskaltdcpp-daemon` and use Web UI for ruling it (search files, add them to downloads queue, etc.).

## Development

During the development EiskaltDC++ in past years we have used different CVS (Subversion first and then Git) and different development models. Currently the process looks like this:

* All development of is done in git `work` branch or special (feature) branches detached from `work` branch.
* Change log file should be updated together with changes in source code. It may be done in a same git commit or in a separate git commit depending on situation. Just use common sense for this. (There were no rule of updating change log in the past which leads to significant delaying of stable releases.)
* Once the changes from `work` branch are ready for usage and build of program is tested for most important systems (Linux, macOS, Windows) they may be merged to `master` branch.
* Daily builds of program for testers, active users and just curious people should be done from git `master` branch.
* Version scheme for builds from git snapshots should look like: `<major>.<minor>.<patch>-<commits>-g<hash>` (where `<major>`, `<minor>` and `<patch>` are not digits but numbers). `<major>.<minor>.<patch>` is last git tag (for stable release), `<commits>` &ndash; the number of commits since last git tag and `<hash>` &ndash; short hash of current git commit.
* Once there is noticeable amount of changes since last stable release or if there are very important bug fixes which should be quickly delivered to users new git tag (`v<major>.<minor>.<patch>`) is created and tarballs with sources are uploaded to SourceForge.
* There is no strict limitation of type of changes suitable for new releases: even "minor" or "patch" version may contain new features and significant changes in GUI.
* In case of noticeable changes in Core of program (library libeiskaltdcpp) the `<major>` part of program version should be changed.
* In case of significant changes (for example, total code refactoring) in any part of program the `<major>` part of program version should be changed.
* In case when where are very few changes since last stable release, but they are important and should be quickly delivered to users the `<patch>` part of program version should be changed.

During development all changes are tested on Continuous Integration services [Travis CI](https://travis-ci.org/eiskaltdcpp/eiskaltdcpp) and [Sibuserv CI](https://sibuserv-ci.org/projects/eiskaltdcpp).

## Developers

### Main developers

* Main developers of EiskaltDC++ are listed in [AUTHORS](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/AUTHORS) file.
* Program contains source code from other free and and open-source projects. All copyright information from them is pedantically documented in [special file](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/full.copyrights.info.in.Debian.style).
* Part of program with GTK+ UI contains additional [Credits.txt](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/eiskaltdcpp-gtk/Credits.txt) file. (This is just a list of contributors from LinuxDC++ and FreeDC++ projects, not all of them are copyright holders.)

### Other contributors

There are a lot of people who were involved into EiskaltDC++ development. Some of them are listed in license headers in source files, some of them might be found only in the history of commits in our git repository. Also there are [translators](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/AUTHORS), testers and just active users. We are thankful to all them!

## How you can help

### Bug reports

If you found a bug please report about it in our [Bug Tracker](https://github.com/eiskaltdcpp/eiskaltdcpp/issues).

### Beta testing

You may use daily builds of program or program compiled from sources by yourself (from git `master` branch) for testing and suggesting of new features, and for reporting about new bugs (if they happen).

### Comments and wishes

We like constructive comments and wishes to functions of program. You may contact with us in public chat room for discussing of your ideas. Some of them will be drawn up as feature requests in our [Bug Tracker](https://github.com/eiskaltdcpp/eiskaltdcpp/issues).

### Translations

The work of translators is quite routine and boring. People who do it usually lose interests and their translations become incomplete. If you see such situation for translation to your native language, please join to our [translations team](https://www.transifex.com/tehnick/eiskaltdcpp/). It is extremely welcome!

Some useful notes about translation process you may find at [special wiki page](https://github.com/eiskaltdcpp/eiskaltdcpp/wiki/Translations).

### Graphics

There are many ways to contribute to the EiskaltDC++ project, if you think you can do a better job with any of the EiskaltDC++ graphics, then go right ahead!

### Programming

Patches are welcome! Contact to EiskaltDC++ developers if you are working on them.

### Packaging

Currently we have active package maintainers for Debian and Ubuntu distros, for macOS and MS Windows. If you have suggestions about improving of packaging, just contact with them.

If you want to prepare personal builds of EiskaltDC++ for MS Windows and macOS systems, it is very welcome! We may add links to them into our documentation. Becoming an official maintaner for these systems is more complicated, but also possible.

For other GNU/Linux and *BSD systems the situation is quite clear: just update packages (pkgbuilds, ebuild, etc.) in official repositories of your favorite distributions or make a Personal Package Archive (PPA) with them. We will add links to them into our documentation.

### Donations

We do not accept donations for EiskaltDC++ project as a whole, because there are no expenditures for infrastructure and such like. (Thanks to owners of GitHub, SourceForge, Transifex and Travis CI services for their support of FOSS projects!)

But you may send donations to project contributors (developers, maintainers, translators, etc.) on personal basis. Just contact with them using the contact information from [AUTHORS](https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/AUTHORS) file.

## Packages and installers

* [Official PPA for Ubuntu and distros based on it](https://launchpad.net/~tehnick/+archive/ubuntu/direct-connect) (stable releases)
* [Official PPA for Ubuntu and distros based on it](https://launchpad.net/~tehnick/+archive/ubuntu/direct-connect-devel) (daily builds)
* [Official Snap packages for Ubuntu and other distros](https://snapcraft.io/eiskaltdcpp) (daily builds)
* [Official builds for Windows](https://sourceforge.net/projects/eiskaltdcpp/files/Windows/)
* [Official builds for macOS](https://sourceforge.net/projects/eiskaltdcpp/files/macOS/)
* [Official packages in Debian](https://tracker.debian.org/pkg/eiskaltdcpp)
* [Official packages in Ubuntu](https://launchpad.net/ubuntu/+source/eiskaltdcpp)
* [Official packages in Fedora](https://apps.fedoraproject.org/packages/s/eiskaltdcpp)
* [Official packages in Gentoo Linux](https://packages.gentoo.org/packages/net-p2p/eiskaltdcpp)
* [Official packages in ALT Linux](http://www.sisyphus.ru/ru/srpm/eiskaltdcpp)
* [Official packages in Haiku](https://depot.haiku-os.org/eiskaltdcpp)
* [Official packages in FreeBSD](https://www.freshports.org/search.php?query=eiskaltdcpp)
* [Packages for openSUSE](https://software.opensuse.org/package/eiskaltdcpp) (official and unofficial)
* [Packages for Arch Linux](https://aur.archlinux.org/packages.php?K=eiskaltdcpp) (in AUR)
* [Packages for different Linux distros](https://repology.org/metapackage/eiskaltdcpp/versions) (the best aggregator of links)
* [Packages for different Linux distros](https://pkgs.org/download/eiskaltdcpp) (alternative aggregator of links)
* [Unofficial PPA for Debian and Ubuntu](https://notesalexp.org/index-old.html) (maintained by Alex_P)

## Extra links

* [Project statistics on GitHub](https://github.com/eiskaltdcpp/eiskaltdcpp/graphs/contributors)
* [Project statistics on OpenHub](https://www.openhub.net/p/eiskaltdcpp)
* [Project FreeDC++ (was basis of eiskaltdcpp-gtk)](https://github.com/eiskaltdcpp/freedcpp) (sources)
* [Official Web UI for eiskaltdcpp-daemon](https://github.com/eiskaltdcpp/eiskaltdcpp-web) (sources)
* [Alternative Web UI for eiskaltdcpp-daemon ](https://github.com/eiskaltdcpp/icecult) (sources)
* [Old screenshots](https://tehnick.github.io/eiskaltdcpp/en/screenshots.html) (in English)
* [Old screenshots](https://tehnick.github.io/eiskaltdcpp/screenshots.html) (in Russian)
* [Old notes about EiskaltDC++](https://tehnick.github.io/eiskaltdcpp/) (in Russian)
* [Old notes about FreeDC++](https://tehnick.github.io/freedcpp/) (in Russian)
* [Old FAQ about EiskaltDC++](https://tehnick.github.io/eiskaltdcpp/faq.html) (in Russian)
* [Setting up interception of magnet links from web browsers](https://tehnick.github.io/dc_clients/magnet-links.html) (in Russian)
* [Review of DC clients for Linux](https://tehnick.github.io/dc_clients/) (in Russian)
* [DC++ and programs based on it at Wikipedia](https://en.wikipedia.org/wiki/DC%2B%2B) (in English)
* [DC++ and programs based on it at Wikipedia](https://ru.wikipedia.org/wiki/DC%2B%2B) (in Russian)
* [Valknut and programs based on it at Wikipedia](https://en.wikipedia.org/wiki/Valknut_\(software\)) (in English)
* [EiskaltDC++ at Wikipedia](https://ru.wikipedia.org/wiki/EiskaltDC%2B%2B) (in Russian)


Have fun!


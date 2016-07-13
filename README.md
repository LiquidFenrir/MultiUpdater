# MultiUpdater
or µpdtr for short, is an updater/downloader for 3ds applications/a9lh payloads/various files written in C

# Configuration
The configuration file is named `config.json` and placed in `/3ds/MultiUpdater/`.  
An example can be found in the file config.json present in the repo

# Dependencies / Requirements
This project requires ctrulib, zlib, and jansson to be installed on your computer for you to be able to build it.
zlib and jansson can be installed with [3ds_portlibs](https://github.com/devkitPro/3ds_portlibs)

# Compilation
Just run `make` in a terminal.
`make cia` will require that bannertool and makerom are somewhere in your PATH
There **WILL** be compilation warnings, not caused by my code but by minizip.

# License
MultiUpdater is licensed under the MIT license, a copy of which can be found in the [LICENSE file](../blob/master/LICENSE).  
The files located in `source/minizip` are part of [minizip](https://github.com/nmoinvaz/minizip) and are absolutely not created by me. They are licensed under the zlib license, a copy of which can be found in the [minizip-LICENSE file](../blob/master/minizip-LICENSE).

# Credit
all of the ctrulib contributors, for ctrulib which this depends upon  
Makefile by Hamcha  
banner audio by [Akrythael on freesound](https://www.freesound.org/people/Akrythael/sounds/334919/)  
\#Cakey on freenode for help
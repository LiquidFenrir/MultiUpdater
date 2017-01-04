# MultiUpdater
MultiUpdater is an updater for a9lh payloads and other files on the SD card. The ability to update CIA applications is planned but has not been implemented.

# Configuration
The configuration file is named `config.json` and should be placed in `/3ds/MultiUpdater/`.  
An example config can be found in the repo.

# Dependencies / Requirements
This project requires ctrulib, zlib, and jansson in your environment.
zlib and jansson can be installed with [3ds_portlibs](https://github.com/devkitPro/3ds_portlibs)

# Compilation
Just run `make` in terminal. Bannertool and makerom **must** be in your PATH for or CIA compilation will fail.
There **WILL** be compilation warnings, not caused by my code but by minizip.

# License
MultiUpdater is licensed under the MIT license, a copy of which can be found in the [LICENSE file](../blob/master/LICENSE).  
The files located in `source/minizip` are part of [minizip](https://github.com/nmoinvaz/minizip) and are absolutely not created by me. They are licensed under the zlib license, a copy of which can be found in the [minizip-LICENSE file](../blob/master/minizip-LICENSE).

# Credits
All of the ctrulib contributors, for ctrulib which this depends upon  
Makefile by Hamcha  
\#Cakey on freenode for help

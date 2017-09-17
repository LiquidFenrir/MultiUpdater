# MultiUpdater
MultiUpdater is an updater for FIRM payloads, CIA applications, and other files on the SD card.

# Configuration
The configuration file is named `config.json` and should be placed in `/3ds/MultiUpdater/`.  
An example config can be found in the repo.

# Dependencies / Requirements
This project requires ctrulib, zlib, bzip2, xz, and libarchive.
zlib, bzip2, xz, and libarchive can be installed with [3ds_portlibs](https://github.com/LiquidFenrir/3ds_portlibs/tree/libarchive)

# Compilation
Just run `make` in terminal. Bannertool and makerom **must** be in your PATH or CIA compilation will fail.

# License
MultiUpdater is licensed under the MIT license, a copy of which can be found in the [LICENSE file](../blob/master/LICENSE).  

# Credits
All of the ctrulib contributors, for ctrulib which this depends upon  
\#Cakey on freenode for help

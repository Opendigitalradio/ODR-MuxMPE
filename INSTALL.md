Heres how to install ODR-MuxMPE on your host:

# Compiling manually

## Dependencies
### Debian or Ubuntu 22.04-based OS:
```
# Required packages
## C++11 compiler

    sudo apt-get install --yes build-essential automake libtool

## Boost 1.48 or later

    sudo apt-get install --yes libboost-system-dev

## For SRT functionality install libsrt

    git clone https://github.com/Haivision/srt.git
    mkdir build
    cd build
    cmake ../
    make
    make install

## tsduck
Compile and download from source.

    git clone https://github.com/tsduck/tsduck.git
    sudo make NODEKTEC=1 NOVATEK=1 NOPCSC=1 install


## Compilation
The *master* branch in the repository always points to the
latest release. If you are looking for a new feature or bug-fix
that did not yet make its way into a release, you can clone the
*next* branch from the repository.

1. Clone this repository:
   ```
   # stable version:
   git clone https://github.com/Opendigitalradio/ODR-MuxMPE.git

   # or development version (at your own risk):
   git clone https://github.com/Opendigitalradio/ODR-MuxMPE.git -b next
   ```
1. Configure the project
   ```
   cd ODR-MuxMPE
   ./bootstrap.sh
   ./configure
   ```
1. Compile and install:
   ```
   make
   sudo make install
   ```

Notes:
- It is advised to run the bootstrap and configure steps again every time you pull updates from the repository.
- The configure script can be launched with a variety of options. Run `./configure --help` to display a complete list


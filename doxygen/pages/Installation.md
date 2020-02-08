# Installation

[TOC]

This document assumes that the **Arduino IDE** is already installed, as well as 
the necessary hardware support packages, such as Teensyduino or the ESP32 Core, 
if you're not using a standard board.  
Here are some links to installation instructions for the Arduino IDE 
on [**Linux**](https://tttapa.github.io/Pages/Ubuntu/Software-Installation/Arduino/Arduino-IDE.html),
on [**Windows**](https://www.arduino.cc/en/guide/windows), 
and on [**OSX**](https://www.arduino.cc/en/guide/macOSX).

If you want to keep up to date with the latest developments, or if you want an 
easy way to update, the Git install is recommended. Otherwise, the installation
without Git will be fine as well.

## Installation of the Arduino library without Git

### 1. Download

Download the repository as a ZIP archive by going to the [home page of the 
repository](https://github.com/tttapa/Arduino-KVComm) and click
the green _Clone or download_ button on the top right.

If you want to download the latest stable release, you can go to the 
[Releases page](https://github.com/tttapa/Arduino-KVComm/releases) on GitHub.

### 2. Install the Library in the Arduino IDE

Open the Arduino IDE, and go to the _Sketch &gt; Include Library &gt; Add .ZIP
Library_ menu.  
Then navigate to your downloads directory where you just downloaded the 
library.  
Select it, and click _Ok_.

## Installation of the Arduino library using Git

### 0. Install Git

If you haven't already, install Git from https://git-scm.com/downloads or use 
your system's package manager.

On Ubuntu and other Debian-based distros: 
```sh
sudo apt install git
```

### 1. Browse to your Arduino Libraries folder

Open a terminal window and change the directory to your Arduino folder.

In a Linux terminal or Git Bash: 
```sh
mkdir -p ~/Arduino/libraries && cd $_
```

### 2. Clone the Library

```sh
git clone https://github.com/tttapa/Arduino-KVComm.git
```

### 3. Updating to the latest version

If you installed the library using Git, you can easily update it when a new 
version comes out.  
To update to the latest `master` version:

```sh
git pull
```

## Installation of the C++ library

After installing the Arduino library, you can also install the C++ library for
your computer.

Open the `Arduino-KVComm/build` folder in a terminal, and run the following
commands to build, test, and install the library:

```sh
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(($(nproc) * 2))
make test
sudo make install
```

If you don't have `sudo` access, you can install it for your user only, instead
of system wide:

```sh
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$HOME/.local
make -j$(($(nproc) * 2))
make test
make install
```

Keep in mind that the `~/.local/lib` folder may not be in the path of the 
dynamic linker/loader, or in the CMake path, so you might have to add the paths
manually.
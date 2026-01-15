<div align="center">

[![Arch Linux](https://img.shields.io/badge/Arch%20Linux-1793D1?logo=arch-linux&logoColor=fff)](https://archlinux.org/)
[![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?logo=ubuntu&logoColor=white)](https://ubuntu.com/download)
[![C](https://img.shields.io/badge/C23-00599C?logo=c&logoColor=white)](#)
[![Dependencies](https://img.shields.io/badge/Dependencies-zero-2EA44F)](#)
[![Size](https://img.shields.io/github/repo-size/Wimers/signals?color=blue)](#)

</div>

<p align="center">
  <img src="images/graphics/bannerCompr.png" width="100%" />
</p>

---

## Overview

**Signals** is a BMP image processor built with zero dependencies for Linux/Unix systems and features a range of image editing tools, custom filters/effects, ANSI terminal rendering, and image exporting.

### Key Features

- **Effects:** Channel isolation, colour scaling, contrast, inversion, and more!
- **Transformations:** Image rotation, reversing, and flipping.
- **Steganography**: Encode secret messages for later decoding.
- **Blending:** Combine and merge images.

> Currently only supports 24-bit Windows BMP's.

### Visual Tuning Tips

- **Terminal Rendering**:
  - For the best viewing experience, zoom out in your terminal.
  - Useful for checking parameters without saving to disk.

- **Cascading Operations**:
  - You can combine flags to process multiple effects simultaneously.

---

## Examples

### Heart

<p align="center">
  <img src="images/original/heartOriginal.bmp" width="49%"/>
  <img src="images/modified/heart.bmp" width="49%"/> 
</p>

> ```bash
> $ signals -i images/original/heartOriginal.bmp -o heart.bmp --glitch 60 --brightness-cut 245
> ```

---

### Melt </3

<p align="center">
  <img src="images/modified/dbmelt.bmp" width="1000" height="400" />
</p>

> ```bash
> $ signals -i images/original/heartOriginal.bmp -o temp.bmp --reverse  --melt -1 --contrast 30
> $ signals -i images/modified/heart.bmp -o dbmelt.bmp --merge temp.bmp --melt  1 --contrast 30
> ```

---

### Voyager

<p align="center">
  <img src="images/graphics/turtleOriginal.jpg" width="49%" />
  <img src="images/graphics/turtle.jpg" width="49%" />
</p>

> ```bash
> $ signals -i images/original/turtle.bmp -o tmp.bmp --scale-strict "1.00, 0.60, 0.6"
> $ signals -i tmp.bmp -o turtle.bmp --contrast 2.30 --scale-strict "2.30, 1.15, 1.0"
> ```

---

### InfraBW

<p align="center">
  <img src="images/original/birdsIndia.bmp" width="49%" />
  <img src="images/modified/infrabirds.bmp" width="49%" />
</p>

> ```bash
> $ signals -i images/original/birdsIndia.bmp -o infrabird.bmp -g -v --scale-strict "1.7, 1.7, 1.7" --contrast 15
> ```

---

### Fortune

<p align="center">
  <img src="images/original/cat.jpg" width="49%" />
  <img src="images/modified/cat.jpg" width="49%" />
</p>

> ```bash
> $ signals -i images/original/cat.bmp -o cat.bmp --scale "1.8, -1, -1" --swap -C 1.5 -G 10
> ```

---

## Usage

To view a list of all available program commands and flags, see:
```bash
$ signals help
```

Similarly, usage for individual commands may also be viewed by specifying the command:
```bash
$ signals help [command]
```
> Note: Aside from the help command, all operations require an input file to be specified via `-i` or `--input`.

### **I/O**
| Flag | Long Flag | Argument | Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| `-i` | `--input` | `<file>` | `.bmp` | Input BMP file path. |
| `-o` | `--output` | `<file>` | `Any` | Output file path. |
| `-m` | `--merge` | `<file>` | `.bmp` | Averages the pixel data of the two images together. |
| `-c` | `--combine` | `<file>` | `.bmp` | Overlays a second image onto the input. |
| `-d` | `--dump` | | | Dumps the BMP header data to the terminal. |
| `-p` | `--print` | | | Renders the image to the terminal. |
| `-e` | `--encode` | `<file>` | `.bmp` | Embeds contents of a file into an image. |

### **Filters**
| Flag | Long Flag | Argument | Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| `-f` | `--filter` | `<channels>` | `char` | Isolate specific channels (e.g. 'RB') |
| `-h` | `--hue` | `<R, G, B>` | `int` | Increase/decrease RGB channel intensity by a constant (-255 ↔ 255). |
| `-g` | `--grayscale` | | | Converts to black & white (Luma). |
| `-a` | `--average` | | | Convert to grayscale (Average Intensity). |
| `-v` | `--invert` | | | Inverts all colours (Negative effect). |
| `-s` | `--swap` | | | Swaps the red and blue colour channels. |
| `-C` | `--contrast` | `<factor>` | `float`| Adjusts contrast intensity. |
| `-b` | `--brightness-cut` | `<0-255>` | `uint8_t` | Zeros pixel colour if value exceeds cutoff. |
| `-T` | `--scale-strict` | `<R, G, B>` | `float` | Scale R, G, B channels by respective multipliers. |

### **Effects**
| Flag | Long Flag | Argument | Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| `-M` | `--melt` | `<offset>` | `int32_t` | Pixel Sorting Effect (negative input to invert). |
| `-G` | `--glitch` | `<offset>` | `size_t` | Apply horizontal shift effect to red and blue channels. |
| `-S` | `--scale` | `<R, G, B>` | `float` | Scale R, G, B channels by respective multipliers, with integer overflow allowed. |
| `-B`,| `--blur` | `<radius>`| `size_t` | Blurs the image using the set radius. |

### **Geometry**
| Flag | Long Flag | Argument | Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| `-r` | `--rotate` | `<N>` | `long` | Rotates image 90° clockwise `N` times. |
| `-t` | `--transpose` | | | Tranposes the image. |
| `-R` | `--reverse` | | | Reverse image horizontally. |
| `-F` | `--flip` | | | Flips image vertically. |

## Prerequisites

This project utilizes **C23** features.

- **GCC**: requires v14.0 or higher.
  - If unsure see `gcc --version` for your current version.
- **Make**: build system.
- **SDL2**: *(optional)* for image rendering via a GUI.

## Install Dependencies

### Arch

```bash
$ sudo pacman -S base-devel
$ sudo pacman -S sdl2   # (optional)
```

### Ubuntu

```bash
$ sudo apt update
$ sudo apt install build-essential
$ sudo apt install libsdl2-dev   # (optional)
```

Check **GCC** version:
```bash
$ gcc --version
```

If **GCC** version older than v14.0:
```bash
# Add the toolchain repo (Only required for Ubuntu 22.04 or older)
$ sudo add-apt-repository ppa:ubuntu-toolchain-r/test$
$ sudo apt update

# Install GCC 14
$ sudo apt install gcc-14

# Set it as the default compiler
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 --slave /usr/bin/g++ g++ /usr/bin/g++-14
```

## Linux Build instructions

### Clone

```bash
$ git clone https://github.com/wimers/signals.git
$ cd signals
```

### Install

For standard build:

```bash
$ sudo make install
```

For **SDL2** build:
```bash
$ sudo make sdl-install
```

> If you intend to modify source code, consider using a symbolic link via `sudo make link`.

### Cleanup (optional)
```bash
$ cd .. && rm -rf signals
```

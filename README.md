<div align="center">

[![Arch Linux](https://img.shields.io/badge/Arch%20Linux-1793D1?logo=arch-linux&logoColor=fff)](https://archlinux.org/)
[![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?logo=ubuntu&logoColor=white)](https://ubuntu.com/download)
[![C](https://img.shields.io/badge/C23-00599C?logo=c&logoColor=white)](#)
[![Dependencies](https://img.shields.io/badge/Dependencies-zero-2EA44F)](#)
[![Size](https://img.shields.io/github/repo-size/Wimers/signals?color=blue)](#)

</div>

# BMP Image Processor

<img src="images/modified/dbmelt.bmp" align="right" width="420" height="260" />

A BMP image processor built with zero dependencies for Linux/Unix systems. Features a range of image editing tools, custom filters/effects, ANSI terminal rendering, and image exporting.

**Features:**
- **Effects:** Channel isolation, colour scaling, contrast, inversion, and more!
- **Transformations:** Image rotation, reversing, and flipping.
- **Blending:** Combine and merge images.

> Currently only supports 24-bit Windows BMP's.

<div style="clear: both;"></div>

---

## Visual Tuning Tips

- Terminal Rendering:
  - For the best viewing experience, zoom out in your terminal.
  - Useful for checking parameters without saving to disk.

- Cascading Operations:
  - You can combine flags to process multiple effects simultaneously.

---

# Heart
```console
$ ./bmp -i images/original/heartOriginal.bmp -o heart.bmp --glitch 60 --brightness-cut 245
```

<p align="center">
  <img src="images/original/heartOriginal.bmp" width="49%"/>
  <img src="images/modified/heart.bmp" width="49%"/> 
</p>

# Melt </3
```console
$ ./bmp -i images/original/heartOriginal.bmp -o temp.bmp --reverse  --melt -1 --contrast 30
$ ./bmp -i images/modified/heart.bmp -o dbmelt.bmp --merge temp.bmp --melt  1 --contrast 30
```

<p align="center">
  <img src="images/modified/dbmelt.bmp" width="1000" height="400" />
</p>

---

# Usage

### **I/O**
| Flag | Long Flag | Argument | Description |
| :--- | :--- | :--- | :--- |
| `-h` | `--help` | | Shows a helpful program usage message. |
| `-i` | `--input` | `<file>` | Input BMP file path. |
| `-o` | `--output` | `<file>` | Output file path. |
| `-d` | `--dump` | | Dumps the BMP header data to the terminal. |
| `-p` | `--print` | | Renders the image to the terminal. |

### **Filters**
| Flag | Long Flag | Argument | Description |
| :--- | :--- | :--- | :--- |
| `-g` | `--grayscale` | | Converts to black & white (Luma). |
| `-a` | `--average` | | Convert to grayscale (Average Intensity). |
| `-s` | `--swap` | | Swaps the red and blue color channels. |
| `-v` | `--invert` | | Inverts all colors (Negative effect). |
| `-S` | `--scale` | `<val>` | Colour intensity multiplier |
| `-t` | `--contrast` | `<val>` | Adjusts contrast intensity (0-255). |
| `-m` | `--dim` | `<val>` | Reduce pixel intensity (0-255). |
| `-b` | `--brightness-cut` | `<val>` | Zeros pixel colour if value exceeds cutoff (0-255). |

### **Effects**
| Flag | Long Flag | Argument | Description |
| :--- | :--- | :--- | :--- |
| `-M` | `--melt` | `<offset>` | Pixel Sorting Effect (negative input to invert). |
| `-l` | `--glitch` | `<offset>` | Apply horizontal shift effect to red and blue channels. |
| `-G` | `--merge` | `<file>` | Overlays a second image onto the input. |
| `-c` | `--combine` | `<file>` | Averages the pixel data of two images together. |

### **Geometry**
| Flag | Long Flag | Argument | Description |
| :--- | :--- | :--- | :--- |
| `-u` | `--flip` | | Flips image vertically. |
| `-r` | `--reverse` | | Reverse image horizontally. |
| `-w` | `--rotate` | `<N>` | Rotates image 90° clockwise `N` times. |

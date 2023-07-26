# Magnifiqus

## Overview

Magnifiqus is a screen magnifier made with [Qt][1].

![Screenshot](resources/screenshot.png)

## Dependencies

Runtime:

- Qt5/6 base

Build:

- CMake
- Qt Linguist Tools
- Git (optional, to pull latest VCS checkouts)

## Build

`CMAKE_BUILD_TYPE` is usually set to `Release`, though `None` might be a [valid alternative][2].<br>
`CMAKE_INSTALL_PREFIX` has to be set to `/usr` on most operating systems.<br>
Using `sudo make install` is discouraged, instead use the system package manager where possible.

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr -W no-dev
cmake --build build --verbose
DESTDIR="$(pwd)/package" cmake --install build
```

## Packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/magnifiqus.svg)][3]


[1]: https://qt.io
[2]: https://wiki.archlinux.org/title/CMake_package_guidelines#Fixing_the_automatic_optimization_flag_override
[3]: https://repology.org/project/magnifiqus/versions

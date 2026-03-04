# NormalMap

A Nuke NDK plugin that generates per-pixel normal vectors from a monochrome input image (depth/height map) using a Sobel filter.

**Original author:** Jonathan Egstad

**Original Nukepedia page:** [NormalMap on Nukepedia](https://www.nukepedia.com/tools/plugins/filter/normalmap/)

## Description

NormalMap accepts a depth channel (typically a heightfield image) and outputs a per-pixel normal based on perceived contours. It uses a Sobel filter to determine the surface orientation, making it useful for relighting, shading, and other normal-based compositing operations directly within Nuke.

## Building from Source

### Requirements

- CMake 3.16+
- Nuke NDK (matching your target Nuke version)
- C++17 compatible compiler

### Build

```bash
mkdir build && cd build
cmake .. -DNUKE_INSTALL_PATH=/path/to/Nuke17.0
cmake --build . --config Release
```

### Install

Copy the resulting `NormalMap.so` (Linux), `NormalMap.dylib` (macOS), or `NormalMap.dll` (Windows) to your `~/.nuke` directory or any path on `NUKE_PATH`.

## Usage

1. Add a **NormalMap** node after a depth or heightfield input.
2. The plugin outputs a per-pixel normal map derived from the contours in the image.
3. Feed the result into relighting setups, shading networks, or any workflow that requires surface normals.

## License

This project is licensed under the **GNU General Public License v3.0 or later** — see the [LICENSE](LICENSE) file for details.

Copyright (c) 2011, Jonathan Egstad.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

## Credits

- **Jonathan Egstad** — original author and all plugin logic.
- **Peter Mercell** — Nuke 17 compilation and distribution.

# snrkos

The snrkos game engine. It includes code derived from the `cards-client`
[codebase](https://github.com/rmkrupp/cards-client).

This project has submodules. When you clone it run `git submodule update
--init`.

Build with `./configure.py --build=release && ninja`

## Dependencies

### Build

 - gcc etc.
 - ninja
 - python

### Runtime

 - vulkan (on ArchLinux, building requires `vulkan-devel`)
 - GLFW
 - liblzma (xz utils)
 - OpenMP

Pass `--disable-argp` to `configure.py` to fall back to getopt. This is the
default if `--build=w64` is given.

OpenMP is required to build the `generate-dfield` tool. You can pass
`--disable-tool=generate-dfield` to avoid building this tool and therefore
avoid the OpenMP dependency.

## Project Goals

TODO

# TG2027 GBA Demo

Development bootstrap for **TG2027 – A Machine From 2001. A Demo From 2027.**
The cartridge build is defined by `Makefile`; CMake is for CLion source
navigation only.

## Build

Install Podman, then run:

```sh
./build.sh
```

The script builds the pinned `devkitpro/devkitarm:20260610` image and invokes
the Makefile inside it. The ROM is written to `rom/tg2027.gba`. To print the
linker section sizes and remaining space in the 32 MiB cartridge ROM budget:

```sh
./build.sh rom-size
```

To remove generated build products:

```sh
./build.sh clean
```

## Current ROM

The Mode 3 title screen displays the logo from
`assets/graphics/tg_logo.png` as a 4bpp, 64x64 hardware OBJ sprite. The
prepared asset is an indexed 4-bit PNG with a transparent black index and at
most 16 palette entries; GNU Make converts it with `grit` to GBA tile data
and RGB555 colors as part of the ROM build. The screen credits **En Byte
til** in its upper-left corner and uses brief colored sprite-band glitches.

The logo PNG is prepared from the supplied image with
`tools/prepare_logo.sh` (ImageMagick required to regenerate it). Open
`rom/tg2027.gba` in mGBA or on hardware to inspect the title screen.

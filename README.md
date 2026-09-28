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
`assets/graphics/tg_logo.png` as a 4bpp, 32x32 hardware OBJ sprite. The
prepared asset is an indexed 4-bit PNG with a transparent black index and at
most 16 palette entries; GNU Make converts it with `grit` to GBA tile data
and RGB555 colors as part of the ROM build. The screen credits **En Byte
til** in its upper-left corner and uses brief colored sprite-band glitches.

The logo PNG is prepared from the supplied image with
`tools/prepare_logo.sh` (ImageMagick required to regenerate it). Open
`rom/tg2027.gba` in mGBA or on hardware to inspect the title screen.

## devkitPro headers for CLion navigation

`CMakeLists.txt` is a source-navigation-only target; it looks for the
libgba headers under `$DEVKITPRO/libgba/include` (or `/opt/devkitpro`
if `DEVKITPRO` is unset). The actual ROM build never uses these files —
`./build.sh` always compiles inside the pinned Podman image — but CLion
needs a local copy on disk to resolve `#include <gba.h>` and friends.

Fetch a local copy straight from the pinned image with Podman:

```sh
./tools/fetch_devkitpro_headers.sh
```

This builds/reuses the `devkitpro/devkitarm:20260610` image, then copies
`/opt/devkitpro/libgba` (headers + libs) out of a throwaway container into
`.devkitpro/libgba` in the project root (pass a different destination as
the first argument if you want it elsewhere). The folder is git-ignored.
Point CLion at it by exporting `DEVKITPRO=<project>/.devkitpro` before
launching the IDE (or reloading the CMake project), or copy the contents
to `/opt/devkitpro` yourself if you'd rather use the default location.

`libgba/include` currently contains:

| Header | Purpose |
| --- | --- |
| `gba.h` | Umbrella header; pulls in the rest of the libgba API |
| `gba_types.h` | Fixed-width integer typedefs (`u8`, `u16`, `u32`, ...) |
| `gba_base.h` | Core memory-map macros, register base addresses, `EWRAM_BSS`/`IWRAM_CODE` attributes |
| `gba_video.h` | Display control registers, video modes, background control |
| `gba_sprites.h` | `OBJATTR`, OAM layout, sprite shape/size encoding |
| `gba_affine.h` | Affine transform structs for backgrounds/sprites |
| `gba_dma.h` | DMA channel registers and `dmaCopy` helpers |
| `gba_interrupt.h` | `irqInit`/`irqEnable`, interrupt vector handling |
| `gba_timers.h` | Hardware timer registers |
| `gba_input.h` | Key input polling (`REG_KEYINPUT`, key masks) |
| `gba_sio.h` | Serial I/O (link cable) registers |
| `gba_sound.h` | Sound channel registers |
| `gba_systemcalls.h` | BIOS call wrappers (`VBlankIntrWait`, `Div`, etc.) |
| `gba_multiboot.h` | Multiboot (link-cable ROM transfer) support |
| `gba_console.h` | Simple text console over a tiled background |
| `gba_compression.h` | BIOS Huffman/LZ77/RLE decompression wrappers |
| `disc.h` / `disc_io.h` / `dldi.h` / `fat.h` / `libfatversion.h` | libfat/DLDI storage-card support (unused by this project) |
| `erapi.h` | EZ-Flash / flash cart API (unused by this project) |
| `fade.h` | Screen fade helper routines |
| `mappy.h` | Mappy tilemap loader helper |
| `maxmod.h` / `mm_types.h` | Maxmod audio library API (unused by this project) |
| `mbv2.h` | Multiboot v2 helper |
| `BoyScout.h` | Legacy filesystem helper (unused by this project) |
| `xcomms.h` / `xcomms_cmd.h` | Cross-console link communication API (unused by this project) |

This project only directly includes `gba.h` (which brings in the video,
sprite, DMA, interrupt, input, and systemcalls headers); the rest of the
list is documented for completeness since they ship in the same
`libgba/include` directory.

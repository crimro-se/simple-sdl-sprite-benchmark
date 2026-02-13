# Simple SDL Sprite Benchmark

A simple 2D sprite performance benchmark application using SDL2 and or SDL3. FPS updates every 3 seconds.


## Controls

Supports keyboard and the first connected gamepad.

Arrows or DPAD:
- **RIGHT** - Increase number of sprites (by 100)
- **LEFT** - Decrease number of sprites (by 100)
- **UP** - Toggle movement enable/disable
- **DOWN** - Toggle a rotation transformation on all sprites

Other:
- **ESC** or **Cirlce Button** - Quit the application


## Results

All @ 480*272

| Device               | SDL | Sprite Format | 100 Sprites FPS | 100 Sprites (Rotated) FPS | 500 Sprites FPS | 500 Sprites (Rotated) FPS |
|----------------------|-----|---------------|-----------------|---------------------------|-----------------|---------------------------|
| PPSSPP (100%)        | 2   | ABGR 1555     | 618             | 490                       | 145             | 110                       |
| PPSSPP (100%)        | 3   | ABGR 1555     | 304             | 244                       | 68              | 53                        |
| Vita 1K (Vita build) | 2   | ABGR 8888     | 469             | 464                       | 165             | 162                       |
| Vita 1K (Vita build) | 3   | ARGB 8888     | 272             | 265                       | 61              | 59                        |

Identical results for the PPSSPP emulator were observed across a broad spectrum of devices (PC, Phone, and an RK3326-based chinese handheld running ArkOS). Of course, you can alter the emulated CPU speed in settings and achieve better results.


## Building

SDL2 or SDL3 is the only real dependency.

**Linux build**:
```bash
make bench2
make bench3
```

**PSP build**: uses cmake. Install the [PSPDEV](https://pspdev.github.io/) toolchain first and restart your terminal.
You can add `-DBUILD_PRX=1 -DENC_PRX=1` to the cmake command to create an EBOOT compatible with unmodified PSPs. Set USE_SDL3 to ON or OFF. If you need to make an iso, find the EBOOT2ISO and UMDGen tools online.

```bash
psp-cmake -B build -DCMAKE_BUILD_TYPE=Release -DUSE_SDL3=OFF .
cd build
make
```

**Vita build**: also uses cmake. Install the [Vita SDK](https://vitasdk.org/) first and restart your terminal. Set USE_SDL3 to ON or OFF.
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DVITA=ON -DUSE_SDL3=OFF .
cd build
make
```

## Modifying

- Consult config.h for various settings you may wish to change.

### Updating the sprite
The sprite is embedded into the build as a header file (sprite_data.h)
After updating the sprite data, you may regenerate this file:
```bash
xxd -i sprite.png > sprite_data.h
```

## Running


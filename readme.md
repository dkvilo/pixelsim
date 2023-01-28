## Building instructions

Application was developed and tested on Linux and Mac (Silicon) platforms. Windows build system support is coming soon.

Application is dinamically linked to SDL2, SDL2_image and SDL2_ttf libraries.
For now, only Linux and Mac builds are supported. Windows build is coming soon.

If you want to modify the code and make your own software, I strongly recommend you to use platform specific IDEs like Visual Studio for Windows, Xcode for Mac and CLion for Linux. And ship the application with the libraries included, or build the application with static linking.

### Linux

```bash
  # Install dependencies (sdl2, sdl2_image, sdl2_ttf)
  sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
  make clean build_linux
```

### Mac (Intel and m1)

```bash
  brew install sdl2 sdl2_image sdl2_ttf
  make clean build_macosx
```

### Windows

- coming soon

## License For Source Code

You are granted a perpetual, irrevocable, non-exclusive, royalty-free license to use, copy, modify, and distribute this software in the form of binary. You may not remove copyright, trademark, or other notices from the source code.

You are allowed to create your own software using this source code. You are not allowed to re sell the source code.

You are allowed to sell your own software that was created using this source code, in a form of binary.

## License For Assets

Please Reffer to the LICENSE.txt file in the assets folder.

## Credits and dependencies

- [SDL2](https://www.libsdl.org/)
- [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)

# Pixel Simulator Studio

Pixel Types:

- Water
- Sand
- Fire

Features for v0.1:

- Dynamic Drawing and Exproting Pixelart as PSB (Pixel Simulator Binary)
- Static Drawing and Exproting Pixelart as PNG
- Import/Export Interactive Frame
- Import PNG as Interactive Frame
- Export Buffer as PNG
- Multiple Frames/Canvas
- Multiple Brushes
- Copy/Paste Frames
- Canvas Zoom
- Tileviewer

Possiable feature list that may will be added in v1.0:

- [ ] More pixel types / Custom Pixel Creation
- [ ] More brushes / Custom Brushe Creation
- [ ] Color picker
- [ ] Layering for frame
- [ ] Lua scripting to define different behaviors for pixels
- [ ] HTML5/Webassembly build support (for exported runtime)
- [ ] Tile editor
- [ ] Text editor for scripting

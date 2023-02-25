# Pixel Simulator - v0.0.1

The goal is to create a pixel art editor that can be used to create interactive pixel art. At this point, The interactive pixel art can be imported/exported as a binary file. Also static pixel art can be exported as PNG.

## Demo Video

[![Pixel Simulator Demo](https://img.youtube.com/vi/5_O9FClM6Xw/0.jpg)](https://www.youtube.com/watch?v=5_O9FClM6Xw)

### Pixel Types

- Water
- Sand
- Fire

### Brush Types

- Eraser (Hold 'e' while drawing on canvas)
- Filled Cercle
- Cross
- Filled Quad
- Quad

### Key Bindings for Demo Application

- **Mouse Left Button** - Drawing on selected canvas
- **E + Mouse Left Button** - Erasing pixels from canvas (You need to Hold on E while drawing)
- **\[** - Zoom in on canvas
- **\]** - Zoom out on canvas
- **Scroll wheel** - change brush size (You can use dedicated buttons on the bottom of the panel as well to change the brush size)
- Loading **PSB** files can be done by just dragging and dropping them on the canvas
- **Space** - Will open Tileset viewer, You can hover over the specific cell to sample the coordinates

### Features for v0.1:

- Dynamic Drawing and Exporting Pixelart as PSB (Pixel Simulator Binary)
- Static Drawing and Exporting Pixelart as PNG
- Import/Export Interactive Frame (PSB)
- Import PNG as an Interactive Frame
- Export Buffer as PNG
- Multiple Frames/Canvas
- Multiple Brushes
- Copy/Paste Frames
- Canvas Zoom
- Tileviewer

### Possible feature list that may be added in v1.0:

- More pixel types / Custom Pixel Creation
- More brushes / Custom Brushe Creation
- Color picker
- Layering for frame
- Lua scripting to define different behaviors for pixels
- HTML5/Webassembly build support (for exported runtime)
- Tile editor
- Text editor for scripting

## Building instructions

The application was developed and tested on Linux and Mac (Silicon) platforms. Windows build system support is coming soon.

The application is dynamically linked to SDL2, SDL2_image, and SDL2_ttf libraries.

For now, only Linux and Mac builds are supported. Windows build is coming soon.

If you want to modify the code and make your own software, I strongly recommend you use platform-specific IDEs like Visual Studio for Windows, Xcode for Mac, and CLion for Linux. And ship the application with the libraries included, or build the application with static linking.

#### Linux

Install dependencies (sdl2, sdl2_image, sdl2_ttf)

```
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
make clean build_linux
```

#### Mac (Intel and m1)

```
brew install sdl2 sdl2_image sdl2_ttf
make clean build_macosx
```

#### Windows

```
Coming soon
```

### License For Assets

Please Refer to the LICENSE.txt file in the assets folder.

### Credits and dependencies

[SDL2 - Simple DirectMedia Layer](https://www.libsdl.org/)

[SDL2 Image](https://www.libsdl.org/projects/SDL_image/)

[SDL2 TTF](https://www.libsdl.org/projects/SDL_ttf/)

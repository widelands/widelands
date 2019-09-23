# Widelands [![Build Status Travis-CI](https://travis-ci.org/widelands/widelands.svg?branch=master)](https://travis-ci.org/widelands/widelands) [![Build Status AppVeyor](https://ci.appveyor.com/api/projects/status/github/widelands/widelands?branch=master&svg=true)](https://ci.appveyor.com/project/widelands-dev/widelands)
Widelands is a free, open source real-time strategy game with singleplayer campaigns and a multiplayer mode.
The game was inspired by Settlers II™ (© Bluebyte) but has significantly more variety and depth to it.

![Widelands Screenshot](https://www.widelands.org/static/img/welcome.jpg)


## License

GPL v2+. Some at assets are released under various Creative Commons licenses – see the respective folders.

## Download

On how to download Widelands, see https://www.widelands.org/wiki/Download/

## Compiling

We support compiling Widelands for Linux, Windows under MSys2, and MacOs with GCC >= 4.8 or Clang/LLVM >= 3.4, though it might work with other compilers too. You will need to install the following dependencies:

*  [libSDL](http://www.libsdl.org/) >=2.0
*  [libSDL_image](http://www.libsdl.org/projects/SDL_image) 
*  [libSDL_mixer](http://www.libsdl.org/projects/SDL_mixer) >= 2.0
*  [libSDL_ttf](http://www.libsdl.org/projects/SDL_ttf) >= 2.0
*  [gettext](http://www.gnu.org/software/gettext/gettext.html) 
* libiconv (on same mirrors as gettext)
*  [zlib](http://www.zlib.net/) 
*  [libpng](http://www.libpng.org/pub/png/libpng.html) 
*  [Boost](http://www.boost.org/) >= 1.48
*  [Python](http://www.python.org) >= 1.5.2
*  [libglew](http://glew.sourceforge.net) or [glbinding](https://glbinding.org/)

You can then compile by running our convenience script.

| Command | Purpose |
| --- | --- |
| `./compile.sh` | Full debug build |
| `./compile.sh -r -w` | Release build |
| `./compile.sh -h` | List available options |

Read more: https://www.widelands.org/wiki/BuildingWidelands/

## Contributing

We have some instructions on how to use Git to help you if you're new to GitHub: https://www.widelands.org/wiki/GitPrimer/

### Code

Before you make any changes to C++ code, we recommend that you install our githook that will automatically
format any changed files when you commit.

1. Install `clang-format`
2. Run `./install-githooks.sh`

We follow the [Google Styleguide](https://google.github.io/styleguide/cppguide.html).

### Scenarios

For scripting scenarios, see https://www.widelands.org/documentation/lua_index/

### Art

For contributing art, see https://www.widelands.org/wiki/GraphicsDevelopment/

### Translations

For contributing translations, see https://www.widelands.org/wiki/TranslatingWidelands/

## Directory Structure

| Directory | Contents |
| --- | --- |
| cmake | Build system and codecheck rules |
| data | The game's data files. Images, sounds, music, scripting, maps, campaigns, tribes, ... |
| debian | Packaging for Debian-based Linux distributions |
| doc | Sphinx [documentation](https://www.widelands.org/documentation/index.html) |
| po | Translation files |
| [src](https://github.com/widelands/widelands/tree/master/src) | C++ source code |
| test | Scripted maps for our regression test suite |
| utils | Diverse utilities: Building translations, code formatting, packaging Mac & Windows, ... |




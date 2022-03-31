# Widelands

[![Build Status AppVeyor](https://ci.appveyor.com/api/projects/status/github/widelands/widelands?branch=master&svg=true)](https://ci.appveyor.com/project/widelands-dev/widelands)
[![Code Quality](https://github.com/widelands/widelands/workflows/Formatting%20your%20code/badge.svg)](https://github.com/widelands/widelands/actions?query=workflow%3A%22Formatting+your+code%22)
[![Build](https://github.com/widelands/widelands/workflows/Build/badge.svg)](https://github.com/widelands/widelands/actions?query=workflow%3ABuild)
[![Mirrored on Launchpad](https://github.com/widelands/widelands/workflows/Launchpad%20Mirror/badge.svg)](https://code.launchpad.net/~widelands-dev/widelands/trunk)

Widelands is a free, open source real-time strategy game with singleplayer campaigns and a multiplayer mode.
The game was inspired by Settlers II™ (© Bluebyte) but has significantly more variety and depth to it.

![Widelands Screenshot](https://www.widelands.org/static/img/welcome.jpg)


## License [![License](https://img.shields.io/github/license/widelands/widelands.svg?color=blue)](COPYING)

GPL v2+. Some assets are released under various Creative Commons licenses – see the respective folders.

## Download

On how to download Widelands, see https://www.widelands.org/wiki/Download/

## Compiling

We support compiling Widelands for Linux, Windows under MSys2, and MacOs with GCC >= 5 or Clang/LLVM >= 6, though it might work with other compilers too. We have more detailed documentation available at: https://www.widelands.org/wiki/BuildingWidelands/


### Dependencies

You will need to install the following dependencies:

*  [libSDL](https://www.libsdl.org/) >=2.0
*  [libSDL_image](https://www.libsdl.org/projects/SDL_image)
*  [libSDL_mixer](https://www.libsdl.org/projects/SDL_mixer) >= 2.0
*  [libSDL_ttf](https://www.libsdl.org/projects/SDL_ttf) >= 2.0
*  [gettext](https://www.gnu.org/software/gettext/gettext.html)
* libiconv (on same mirrors as gettext)
*  [zlib](https://www.zlib.net/)
*  [libpng](http://www.libpng.org/pub/png/libpng.html)
*  [Asio](https://think-async.com/Asio/)
*  [Python](https://www.python.org) >= 1.5.2
*  [libglew](http://glew.sourceforge.net) or [glbinding](https://glbinding.org/)


### Compiling with our convenience script

You can then compile by running our convenience script.

| Command | Purpose |
| --- | --- |
| `./compile.sh` | Full debug build |
| `./compile.sh -r -w` | Release build |
| `./compile.sh -h` | List available options |

When compiling has finished, you can call Widelands with

~~~~
./widelands
~~~~

### Compiling with CMake

You can also call CMake manually:

~~~~
mkdir build
cd build
cmake ..
make
~~~~

When compiling has finished, you can call Widelands with

~~~~
cd ..
mv build/src/widelands .
./widelands
~~~~

We have various CMake options available. For example, to create a release build, call

~~~~
cmake -DCMAKE_BUILD_TYPE=Release ..
~~~~

For using the Ninja build system, call

~~~~
mkdir build
cd build
cmake -G Ninja ..
ninja
~~~~

Depending on the Ninja installation, the last line can also be `ninja-build`.

#### CMake options

Note that CMake options are prefixed with `-D`. These are the available options:

| Name | Values | Default| Function
| --- | --- | --- | --- |
| `CMAKE_BUILD_TYPE` | `Debug`/`Release` | `Debug` | Create a release or debug build |
| `OPTION_ASAN` | `ON`/`OFF` | `ON` for Debug builds /`OFF` for Release builds | Use AddressSanitizer. Switching this off only works once. You will have to clear the `build` directory if you want to switch this off again in a later build. |
| `OPTION_BUILD_TRANSLATIONS` | `ON`/`OFF` | `ON` | Build translations |
| `OPTION_BUILD_CODECHECK` | `ON`/`OFF` | `ON` | Build codecheck. Only available in Debug builds. |
| `OPTION_BUILD_WEBSITE_TOOLS` | `ON`/`OFF` | `ON` | Build website-related tools |
| `OPTION_BUILD_TESTS` | `ON`/`OFF` | `ON` | Build unit tests |
| `CMAKE_INSTALL_PREFIX` | A directory | See [CMake documentation](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html) | Define the target directory for the "install" target, e.g. `-DCMAKE_INSTALL_PREFIX=~/widelands-install`. |
| `WL_VERSION` | A version string | Autodetected from git/bzr, or set by adding a `VERSION` file | Define the Widelands version |
| `USE_XDG` | `ON`/`OFF` | `ON` | Follow XDG-Basedir specification. Only available on Linux. |
| `OPTION_USE_GLBINDING` | `ON`/`OFF` | `OFF` | Use glbinding instead of GLEW |
| `OPTION_GLEW_STATIC` | `ON`/`OFF` | `OFF` | Use static GLEW Library |

#### make/ninja targets

You can add targets to the `make` or `ninja` command, e.g. `make lang` to build only the translations. These are the available targets:

| Name | Function |
| --- | --- |
| `ALL` or no target | Compile everything, up to executable with the settings from the `cmake` call |
| `codecheck` | Run the codechecks |
| `doc` | Generate Doxygen documentation. Currently only with Build Type Debug, but this is easily changed if necessary. |
| `install` | Install into the target dir, this is `/usr/local` per default (you need root privileges!) unless you change it (see CMake options above) |
| `lang` | Generate the translations |

## Contributing

We have some instructions on how to use Git to help you if you're new to GitHub: https://www.widelands.org/wiki/GitPrimer/

### Code

The master branch and open pull requests will be formatted automatically by a GitHub action that runs `clang-format`. When you push to an open pull request, the formatting changes will be pushed back to the branch after about four minutes. Don't forget to run 'git pull' before you push again. Formatting is only triggered the first time you push *after* you opened the PR.

You need to enable *local and third-party actions* in the Actions tab of your fork's settings to enable automatic formatting. If you disable actions, no formatting will take place on your branches.

In order to ensure that our continuous integration suite will work properly for branches in your fork, it is strongly recommended that you create a personal access token for our formatting action:
1. Create a Personal Access Token in your **profile** (https://github.com/settings/tokens). Select the scope `Repo → public_repo` for the new token. The token's note does not matter (use e.g. "Widelands Formatting Bot"). Copy the token's hash.
2. Create a secret in your **widelands fork** (`https://github.com/<username>/widelands/settings/secrets`). The secret must be named `WIDELANDS_FORMAT_TOKEN`. Its value must be the hash of the token you just created.

More information about [Personal Access Tokens may be found here](https://docs.github.com/en/free-pro-team@latest/github/authenticating-to-github/creating-a-personal-access-token). More information about [secrets may be found here](https://docs.github.com/en/free-pro-team@latest/github/automating-your-workflow-with-github-actions/creating-and-using-encrypted-secrets).

We follow the [Google Styleguide](https://google.github.io/styleguide/cppguide.html).

### Scenarios

For scripting scenarios, see https://www.widelands.org/documentation/lua_index/

### Art

For contributing art, see https://www.widelands.org/wiki/GraphicsDevelopment/

### Translations

For contributing translations, see https://www.widelands.org/wiki/TranslatingWidelands/

### Testing

For helping with testing, see https://www.widelands.org/wiki/TestingBranches/

### Triaging Issues

For helping with issue management, see https://www.widelands.org/wiki/TriagingBugs/

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


## Obtaining MacOS and MS-Windows builds and testsuite runs

All pushes to master will be built on AppVeyor. Pull request branches are deployed for MS-Windows using a GitHub action. To obtain MS-Windows builds if you do not wish to open a pull request, temporarily add the name of your branch to the `branches` section in `appveyor.yml`. This will not work if the branch is in a fork though.

All pull request branches as well as master are additionally deployed for MacOS, and a testsuite checks them under various compilers. To obtain MacOS builds or testsuite results, temporarily add the name of your branch to the `branches` section in `.github/workflows/build.yaml`. This *does* work for branches in forks as well.

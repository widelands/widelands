compiling wide lands using msvc 6
---------------------------------

- you will need the sdl headers and lib to compile (and dll to execute); sdl is not included in msvc. sdl can be found at www.libsdl.org

- microsoft visual c++ will (by default) not know how what to do with .cc files. the standard file extension for c++ source is .cc on most unix-based systems. to make your visual c++ understand cc files, read
http://support.microsoft.com/default.aspx?scid=kb;EN-US;q181506
(or search for article Q181506 on microsoft.com support).
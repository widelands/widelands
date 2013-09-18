Manual Testing
==============

The ``test`` directory contains various directories with testing
data/scripts/programs that can be used to manually exercise some subpart of
widelands. These tests are not integrated into any kind of automated test
suite but can be run on a as-needed basis by individual programmers. 

The following directories are currently available:

``compatibility/``
   This directory contains old savegame states from earlier builds. You can load
   them manualle by using the ``--loadgame`` switch on the commandline.

``lua/``
   This directory contains two maps that contain a complete test suite for most
   Lua functionality in the game. You can run the test suite by using:

   .. code-block:: bash
   
      widelands --scenario=test/lua/ts.wmf  or
      widelands --editor=test/lua/ts.wmf

   Both should immediately exit Widelands, the test suite will have printed
   its result to stdout. If a test fails, Widelands will not exit however and
   you should investigate stdout more carefully.

   The other test map in this directory tests persistence (saving/loading) of
   Lua objects. You launch it using

   .. code-block:: bash
   
      widelands --scenario=test/lua/persistence.wmf

   This will immediately save a game as ``lua_persistence.wgf``, then proceed to close Widelands.
   You can now load this savestate using

   .. code-block:: bash
   
      widelands --loadgame=/home/sirver/.widelands/save/lua_persistence.wgf

   The test suite will then be run and check if everything was loaded alright.
   Output will be printed to stdout again and Widelands should exit.
   
``richtext/``
   This contains a test suite for the rich text rendering engine that is
   contained in ``src/graphic/text/``. The test stuff consists of a thin
   python wrapper (using cython) and a stand alone c++ program that can render
   rich text. Compilation is done using SCons, take a look at SConscript for
   details. The ``test.py`` script renders all examples in the ``test``
   sub-directory and checks if they match pixel by pixel. To get everything
   working in this directory, you need SDL, SDL_TTF, cython and numpy.
   





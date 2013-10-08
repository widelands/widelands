Manual Testing
==============

The ``manual_test`` directory contains various directories with testing
data/scripts/programs that can be used to manually exercise some subpart of
widelands. These tests are not integrated into any kind of automated test
suite but can be run on a as-needed basis by individual programmers. 

The following directories are currently available:

``richtext/``
   This contains a test suite for the rich text rendering engine that is
   contained in ``src/graphic/text/``. The test stuff consists of a thin
   python wrapper (using cython) and a stand alone c++ program that can render
   rich text. Compilation is done using SCons, take a look at SConscript for
   details. The ``test.py`` script renders all examples in the ``test``
   sub-directory and checks if they match pixel by pixel. To get everything
   working in this directory, you need SDL, SDL_TTF, cython and numpy.
   





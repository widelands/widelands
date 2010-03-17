Implementation Notes
====================

This section is for developers who want to add more features to the Lua
support in widelands (or that want to track down bugs). This contains notes
about how certain things are implemented and how to add to it. 

.. Note:: 

   This section is not for scenario designers, only for developers
   working directly on widelands.


Where Lua is used
-----------------

TODO: starting conditions of tribes
TODO: scenarios
TODO: Plans for winning conditions
TODO: debug console

Hooks into the Game
-------------------

TODO: difference between Lua and LuaFunction packet

Persistence
-----------

TODO: Map_Scripting_Data_Packet. wl.* is not saved.
TODO: pluto, my changes to it (especially persisting classes)
TODO: map object saver % loader

Classes
-------

TODO: luna, my extensions, properties and methods, load and save
functionalities.

Testing
-------

Lua support is currently tested in two different scenarios. Both life in
``src/scripting/test`` and they work essentially the same: they are normal
scenarios which contain a Lua unittest framework named lunit_ that the 
author agreed to be used in widelands like that. The scripts than use various
Lua functions and check that they do the expected things. 

If you add new features to the Lua support of Widelands, consider also adding
tests in the appropriate places in the test suite. This guarantees that nothing
unexpected happens in scenarios and it will show the most common bugs quite
easily. 

.. _lunit: http://www.nessie.de/mroth/lunit/

ts.wmf
^^^^^^

This is the main test suite that checks for all functionality except for
persistence. It can be run like this from a shell::

   $ ./widelands --scenario=src/scripting/test/ts.wmf

or equivalent under windows. The output of the test suite goes the stdout,
just like the output from widelands. It is therefore sometimes a little
difficult to find the output from the tests. If all tests pass, widelands will
be terminated again and somewhere in the output something like this should be
visible::

   #### Test Suite finished.

   353 Assertions checked. All Tests passed!
   
If the test suite fails, widelands will be kept running and the error message
of the failed test will be visible in the output. This is then a bug and
should be reported.

persistence.wmf
^^^^^^^^^^^^^^^
This is a much shorter script that only checks if the various Lua classes are
correctly saved and reloaded again. First, you need to run it as scenario::

   $ ./widelands --scenario=src/scripting/test/persistence.wmf
       
This will result in the creation of various Lua objects. Widelands will then
immediately safe the game as ``lua_persistence.wgf`` and exit. You can then
load this game::

   $ widelands --loadgame=~/.widelands/save/lua_persistence.wgf

This will check that all objects were loaded correctly. If everything worked
out, the following string will be printed to stdout::

   ################### ALL TEST PASS!

Otherwise an error is printed.



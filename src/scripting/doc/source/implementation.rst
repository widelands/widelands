Implementation Notes
====================

This section is for developers who want to add more features to the Lua
support in widelands (or that want to track down bugs). This contains notes
about how certain things are implemented and how to add to it. 

.. Note:: 

   This section is not for scenario designers, only for developers
   working directly on widelands.


Hooks into the Game
-------------------

The scripting interface is defined in ``scripting/scripting.h``. It consists
of the two classes LuaInterface for running scripts and LuaCouroutine which
wraps a Lua coroutine. The Lua interface is accessed via the lua() function in
Editor_Game_Base. 

There are two game commands for lua: ``Cmd_Lua`` and ``Cmd_LuaFunction``. The
first one is only used to enqueue the initial running of the initialization
scripts in maps (and later on also for win conditions). These scripts are
responsible to start coroutines -- everything from then on is handled via the
second packet: Cmd_LuaFunction. 
When a coroutine yields, it is expected to return the time when it wants to be
reawakened. Widelands wraps this coroutine in a LuaCoroutine object and
enqueues a Cmd_LuaFunction to awake and continue the execution of the
coroutine. When a coroutine ends, it is deleted and Widelands forgets about
it.

When entering a Lua command into the debug console, none of the above mechanisms
are used, instead the string is directly executed via
LuaInterface::interpret_string. This means that this is only run on the
current machine and that means that it could desync network games if it alters
the game state in some way. It is an extremely powerful debug tool though.

Classes
-------

Lua uses the prototype principle for OOP. C++ uses a class based approach.
Starting from a thin wrapper implementation from the Lua users wiki called
Luna_ I implemented a easy wrapper for C++ classes that supports inheritance
and properties. The implementation can be found in ``scripting/luna*``. There
are many examples of wrapped classes in the code, so this is just a short
rundown of what to do:

Write the class, make sure that inheritance matches the one you want to have
in Lua later one (that is L_Immovable must be a child of L_MapObject). 
TODO: get_modulename, LUNA_CLASS_HEAD, Constructors needed?, __persist
__unpersists, structures that define functions and properties.

.. _Luna: http://lua-users.org/wiki/SimplerCppBinding

TODO: luna, my extensions, properties and methods, load and save
functionalities.

Persistence
-----------

TODO: Map_Scripting_Data_Packet. wl.* is not saved.
TODO: pluto, my changes to it (especially persisting classes)
TODO: map object saver % loader


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



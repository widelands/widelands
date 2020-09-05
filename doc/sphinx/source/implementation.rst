.. highlight:: default

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

There are two game commands for lua: ``Cmd_LuaScript`` and
``Cmd_LuaCoroutine``. The first one is only used to enqueue the initial
running of the initialization scripts in maps (and later on also for win
conditions). These scripts are responsible to start coroutines -- everything
from then on is handled via the second packet: ``Cmd_LuaCoroutine``. When a
coroutine yields, it is expected to return the time when it wants to be
reawakened. Widelands wraps this coroutine in a LuaCoroutine object and
enqueues a ``Cmd_LuaCoroutine`` to awake and continue the execution of the
coroutine. When a coroutine ends, it is deleted and Widelands forgets about
it.

When entering a Lua command into the debug console, none of the above mechanisms
are used, instead the string is directly executed via
LuaInterface::interpret_string. This means that this is only run on the
current machine and that means that it could desync network games if it alters
the game state in some way. It is an extremely powerful debug tool though.

Lua Classes
-----------

Lua uses the prototype principle for OOP, that is an Instance of a class knows
how to clone itself. C++ uses a class based approach.  Starting from a thin
wrapper implementation from the Lua users wiki called Luna_ I implemented a
easy wrapper for C++ classes that supports inheritance and properties. The
implementation can be found in ``scripting/luna*``. There are many examples of
wrapped classes in the code, so this is just a short rundown of what to do:

Write the class, make sure that inheritance matches the one you want to have
in Lua later one (that is L_Immovable must be a child of L_MapObject). All
classes must be derived from ``LunaClass``. Each class must then contain a
call to ``LUNA_CLASS_HEAD(klassname)`` in it's public definitions. This
defines static tables for properties and methods and the name for this class. The filling
of this data is usually done in the corresponding implementation file. A small
excerpt for the L_Player class looks like so:

.. code-block:: c++

   const char L_Player::className[] = "Player"; // Name of this class
   const MethodType<L_Player> L_Player::Methods[] = {
      METHOD(L_Player, __eq),  // compare operator
      METHOD(L_Player, place_flag), // a method called place_flag
      {0, 0}, // end of methods table
   };
   const PropertyType<L_Player> L_Player::Properties[] = {
      PROP_RO(L_Player, number), // Read only property
      PROP_RW(L_Player, name), // Read write property
      {0, 0, 0},
   };

Each method and getters and setters must be defined inside the class.
Continuing this example, this would mean Player must at least define the
following public functions to satisfy it's definition tables:

.. code-block:: c++

   int L_Player::__eq(lua_State *);
   int L_Player::place_flag(lua_State *);
   int L_Player::get_number(lua_State *);
   int L_Player::get_name(lua_State *);
   int L_Player::set_name(lua_State *);

.. ** <-- Fixes vims syntax highlighting

Luna Classes need even more boilerplate to work correctly. Firstly, they must
define a function ``get_modulename`` that returns a ``const char *`` which
will be the submodule name where this class is registered. For our example,
Player would return ``"game"`` because it is defined in ``wl.game``.

The class must also define two constructors, one that takes no arguments and
is only used to create an empty class that is created for unpersisting, and a
second one that takes a ``lua_State *`` that is called when the construction
is requested from Lua, that is if ``wl.game.Player()`` is called. Some (most?)
classes can't be constructed from Lua and should then answer with
``report_error()``.

Additionally, a function ``__persist(lua_State *)`` and a function
``__unpersist(lua_State *)`` must be defined. They are discussed in the next
section.

.. _Luna: http://lua-users.org/wiki/SimplerCppBinding

Persistence
-----------

When a savegame is created, the current environment of Lua is persisted. For
this, I used a library called Eris_.

.. _Eris: https://github.com/fnuecke

The global environment is persisted into the file map/globals.dump. Everything
is persisted except of the Lua build-in functions and everything in the ``wl``
table and some of our own global functions. Those are c-functions that can not
be written out to disk portably. Everything else can be saved, that is also
everything in the auxiliary scripts are saved to disk, so save games only
depend on the API defined inside the Widelands.

Coroutines are persisted in their ``Cmd_LuaCoroutine`` package.

Luna classes have to implement two functions to be properly persistable:

``__persist(lua_State *)``
   This function is called with an empty table on top of the stack. It is
   expected that this function stores persistable data into this table. This
   table is then persisted instead of the object. Some convenience macros are
   defined in ``luna.h`` to ease this task (e.g. PERS_UINT32).

``__unpersist(lua_State *)``
   On loading, an instance of the user object is created via the default
   constructor. This function is then called with the table that was created
   by ``__persist()`` as an upvalue (because it is inside a closure). The
   object is then expected to recreate it's former state with this table.
   There are equivalent unpersisting macros defined to help with this task
   (e.g. UNPERS_UINT32).

Widelands reassigns some serial number upon saving and restores them upon
loading. Some Luna classes need this information (for example
:class:`MapObject`). Access is provided via the functions ``get_mol(lua_State
*)`` to the Map_Map_Object_Loader and ``get_mos(lua_State *)`` to the
Map_Map_Object_Saver. These function return 0 when not called in
``__persist`` and ``__unpersist``.

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
This is a much shorter script that only checks if some data is correctly saved
and reloaded again. It is also used to check compatibility of savegames
between different versions. First, you need to run it as scenario::

   $ ./widelands --scenario=src/scripting/test/persistence.wmf

This will result in the creation of various Lua objects. Widelands will then
immediately safe the game as ``lua_persistence.wgf`` and exit. You can then
load this game::

   $ widelands --loadgame=~/.widelands/save/lua_persistence.wgf

This will check that all objects were loaded correctly. If everything worked
out, the following string will be printed to stdout::

   ################### ALL TEST PASS!

Otherwise an error is printed.



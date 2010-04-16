Introduction
============

This documentation describes the usage and possibilities of Lua in Widelands.

Where Lua is used
-----------------

Lua is currently used in the following places:

Scenarios
^^^^^^^^^

The most prominent usage of Lua is in scenarios: All scenario logic is
scripted using Lua. How this works is described in the :ref:`scenario_tutorial`. 

Initializations of Tribes in non-scenario games
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the beginning of non scenario games, the player has a choice to use
different starting conditions. There is the very essential one that just sets
a hq and some starting wares and there are more sophisticated ones. 

Adding new ones is simple. Add the name of the corresponding Lua script in the
``[initializations]`` section of the corresponding tribes ``conf`` file and
copy the Lua script for the new initialization into the tribes scripting
directory. The Lua script should return one function that takes a
:class:`~wl.game.Player` object and does the initialization for this player. 

A small example. Let's assume we want to add an initialization to the
barbarians that just prints foo and does nothing else. We first write the
corresponding Lua script and save it under
``barbarians/scripting/init_foo.lua``.

.. code-block:: lua

   return function(plr) 
      print("Foo for player number:", plr.number)
   end

Now we add the following line to the ``[initializations]`` section of
``tribes/barbarians/conf``::

   init_foo=_Foo initialization

The ``_`` says that this is a string that can be translated into foreign
languages. That's it, now it can be selected when starting new games.

Debug console
^^^^^^^^^^^^^

In widelands debug builds you can open a debug console by pressing ``F6``. You
can enter Lua commands here that act in the global environment: That is if you
are in a scenario you can access the global variables and alter all Lua
objects that are in the global scope. All you have to do is to prefix the
commands with ``lua``:

.. code-block:: lua

   lua print("Hello World!")
   lua hq = wl.game.Player(1).starting_field.immovable -- If this is a normal map
   lua hq:set_workers("builder", 100)

This makes for excellent cheating in debug builds, but note that this is for
debug purposes only -- in network games running Lua commands this way will
desync and therefore crash the game and also replays where you changed the
game state via the debug console will not replay properly. It is very useful
for debugging scenarios though. 


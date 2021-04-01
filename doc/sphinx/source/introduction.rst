Introduction
============

This documentation describes the usage and possibilities of Lua to extend Widelands.

Where Lua is used
-----------------

Lua is currently used in the following places:

Scenarios
^^^^^^^^^

The most prominent usage of Lua is in scenarios: All scenario logic is
scripted using Lua. How this works is described in the :ref:`scenario_tutorial`.

Starting conditions of Tribes in non-scenario games
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the beginning of non scenario games, the player has a choice to use
different starting conditions. There is the very essential one that just sets
a hq and some starting wares and there are more sophisticated ones.

To add a new starting condition see :ref:`Starting Conditions <lua_tribes_tribes_start_conditions>`.

Win conditions
^^^^^^^^^^^^^^

In non player scenarios, win conditions define when one single player has won
a game. The definitions of win conditions is very similar to defining
starting conditions: We have to create a Lua script in
``/data/scripting/win_conditions`` which returns an array with some attributes.

Here is an example of the "Autocrat" win condition:

.. code-block:: lua

   include "scripting/coroutine.lua" -- for sleep
   include "scripting/win_conditions/win_condition_functions.lua"

   push_textdomain("win_conditions")

   include "scripting/win_conditions/win_condition_texts.lua"

   local wc_name = "Autocrat"
   -- This needs to be exactly like wc_name, but localized, because wc_name
   -- will be used as the key to fetch the translation in C++
   local wc_descname = _("Autocrat")
   local wc_version = 2
   local wc_desc = _ "The tribe or team that can defeat all others wins the game! This means the opponents do not have any headquarters, ports or warehouses left."
   local r = {
      name = wc_name,
      description = wc_desc,
      peaceful_mode_allowed = false,
      func = function()
         local plrs = wl.Game().players

         -- set the objective with the game type for all players
         broadcast_objective("win_condition", wc_descname, wc_desc)

         -- Iterate all players, if one is defeated, remove him
         -- from the list, send him a defeated message and give him full vision
         repeat
            sleep(5000)
            check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
         until count_factions(plrs) <= 1

         -- Send congratulations to all remaining players
         broadcast_win(plrs,
               won_game.title,
               won_game.body,{},
               wc_descname, wc_version
         )

      end,
   }
   pop_textdomain()
   return r

Hooks
^^^^^

Hooks are called by widelands when a certain event happened.  They are a
rather recent implementation and therefore still limited. More hooks might be
implemented in the future.

You set a hook by setting a field in the global variable ``hooks`` which must
be a dictionary. The only valid hook currently is the ``custom_statistic``
hook which gives the ability to add one new statistic to the general
statistics menu.  This is used in win conditions (e.g. collectors) and could
also be used in some missions in the future. To define a new statistic, use
something like this:

.. code-block:: lua

   hooks = {}
   hooks.custom_statistic = {
      name = _ "Unchanging statistic",
      pic = "map:mycool_stat_picture.png", -- For the menu button
      calculator = function(p)
         -- Calculate the current value for this player
         return p.number * 20
      end,
   }

Every time widelands samples the current statistics, it will call the
``calculator`` function for each player and expects an unsigned integer value
back.

Debug console
^^^^^^^^^^^^^

In widelands debug builds you can open a debug console by pressing ``F6``. You
can enter Lua commands here that act in the global environment: That is if you
are in a scenario you can access the global variables and alter all Lua
objects that are in the global scope:

.. code-block:: lua

   print("Hello World!")
   map = wl.Game().map
   hq = map.player_slots[1].starting_field.immovable -- If this is a normal map
   hq:set_workers("barbarians_builder", 100)

This makes for excellent cheating in debug builds, but note that this is for
debug purposes only -- in network games running Lua commands this way will
desync and therefore crash the game and also replays where you changed the
game state via the debug console will not work. It is very useful
for debugging scenarios though.

Regression testing infrastructure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The `test` directory in the repository contains the regression test suite. A
test is either a savegame plus a set of Lua scripts (test_*.lua) or a map that
contains in its scripting directory a set of (test_*.lua and/or
editor_test*.lua which are only run in the Editor) files.

Each test starts Widelands using either the `--editor`, `--loadgame` or
`--scenario` switch and additionally, the `--script` switch can be supplied to
run a Lua script directly after the game is ready to take commands.

The tests communicate with the test runner through standard output. If a
script outputs "All Tests passed." the test is considered to pass, otherwise
to fail. Whenever a savegame is written inside a test it is later loaded by
the test runner as an additional test.

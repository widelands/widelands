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

Initializations of Tribes in non-scenario games
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the beginning of non scenario games, the player has a choice to use
different starting conditions. There is the very essential one that just sets
a hq and some starting wares and there are more sophisticated ones.

Adding new ones is simple. All you have to do is to create a new script in
the ``scripting`` directory of a tribe. The script should return a :class:`table`
with two keys: ``name`` is the name for this start condition shown to the user
and ``func`` is the function that will be called. This function takes on
argument, a :class:`~wl.game.Player` for which to create the initial
infrastructure.

A small example. Let's assume we want to add an initialization to the
barbarians that gives the player 80 logs, but not at once but one by one
over a certain time. The corresponding lua script could be named
``barbarians/scripting/sc99_init_logs.lua``. The naming decides over the
order in which the starting conditions are listed. Let's write the script:

.. code-block:: lua

   use("aux", "coroutine")  -- For convenience function sleep()

   -- Set the textdomain, so that all strings given to the _() function are
   -- properly translated.
   set_textdomain("tribes")

   -- Now for the array we must return
   return {
      name = _ "Slow start",  -- the name. This is translated.
      func = function(player)  -- The function to be returned
         -- Let the player build all workers.
         player:allow_workers("all")

         -- Place the hq
         local hq = player:place_building("barbarians_headquarters", player.starting_field)

         -- Now add one log to the hq every 250 ms
         for i=1,80 do
            hq:set_wares("log", hq:get_wares("log") + 1)
            sleep(250)
         end
   end
   }

That's it, now it can be selected when starting new games.

Win conditions
^^^^^^^^^^^^^^

In non player scenarios, win conditions define when one single player has won
a game. The definitions of win conditions is very similar to defining
initializations: We have to create a Lua script in
``/scripting/win_conditions`` which returns an array with ``name``,
``description`` and ``func``. Let's also make up a quick example: The first
player to have 200 logs in his HQ wins the game. All others loose. Save the
following file as ``/scripting/win_conditions/havest_logs.lua``.

.. code-block:: lua

   use("aux", "coroutine") -- for sleep

   -- For translations, textdomain must be win_conditions
   set_textdomain("win_conditions")

   return {
      name = _ "Harvest logs",
      description = _ "The first player with 200 logs wins!",
      func = function()
         -- Find all valid players.
         local plrs = wl.Game().players

         -- Iterate all players, check if he is the winner
         local winner = nil
         local losers = {}
         while not winner do
            sleep(5000) -- we do this every 5 seconds
            for idx,p in ipairs(plrs) do -- iterate the players array
               local hq = p.starting_field.immovable
               if hq:get_wares("log") >= 200 then
                  -- We found the winner
                  winner = p
                  losers = plrs
                  table.remove(losers,idx) -- Remove the winner from the losers table
               end
            end
         end

         -- Send the winner a hurray message, the losers a boo
         for idx,p in ipairs(losers) do
            p:send_message(_"You lost!", _"You lost this game!")
         end
         winner:send_message(_"You won!", _"You won this game!")
      end,
   }

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

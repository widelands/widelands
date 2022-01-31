Introduction
============

This documentation describes the usage and possibilities of Lua to extend Widelands.

.. note::

   Files of type ``.lua`` are very basic text files. Do not use any fancy word
   processor (Word, OpenOffice and their like) because they produce the wrong 
   file format. Make sure to use a plain text editor (like Notepad under 
   Windows, nedit under Linux and TextEdit under Mac OS X).

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
starting conditions which returns an array with ``name``, ``description`` and ``func``.
Take a look at the `endless_game wincondition <https://github.com/widelands/widelands/blob/master/data/scripting/win_conditions/endless_game.lua>`_
for a basic example. Let’s make up a quick example: The first player to have
200 logs in his HQ wins the game. All others loose. Save the following file as 
``data/scripting/win_conditions/harvest_logs.lua``.

.. code-block:: lua

   include "scripting/coroutine.lua"                              -- for sleep()
   include "scripting/win_conditions/win_condition_functions.lua" -- for broadcast_objective()

   -- Some variable which get used throughout this win condition:
   
   local textdomain = "win_conditions"                  -- For translations, textdomain must be win_conditions
   push_textdomain(textdomain)

   local wc_name = "Harvest logs"                       -- The name of win condition, not localized
   local wc_descname = _("Harvest logs")                -- wc_descname has to be exactly like wc_name, because it
                                                        -- will be used as the key to fetch the translation in C++

   local wc_version = 1                                 -- The version of this win condition

   local wc_desc = _("The first player with 200 logs in his headquarter wins!")

   local wc_definition = {                              -- The table defining the win condition
      name = wc_name,
      description = wc_desc,
      peaceful_mode_allowed = true,                     -- Enabling the checkbox for peaceful mode in game setup menu
      func = function()                                 -- The function processing the win condition
         broadcast_objective("win_condition",           -- Set objective with win condition for all players
                             wc_descname, 
                             wc_desc)
         local plrs = wl.Game().players                 -- All players
         local winner = nil                             -- No winner yet
         local losers = {}                              -- Table of loosers
         local map = wl.Game().map                      -- To get access to map objects

         while not winner do                            -- The main loop, it will run as long there is no winner
            sleep(5000)                                 -- Do this every 5 seconds
            for idx, p in ipairs(plrs) do               -- Iterate all players
               
               local sf = map.player_slots[p.number].starting_field    -- The starting field of this player
               local hq = sf.immovable                  -- The headquarters of this player
               if hq:get_wares("log") >= 200 then       -- Check if more than 200 logs are stored
                  winner = p                            -- This player is the winner!
                  losers = plrs                         -- Store all players and ...
                  table.remove(losers,idx)              -- ... remove the winner from the losers table
               end
            end
         end

         push_textdomain(textdomain)                    -- Each part containing localized strings
                                                        -- needs pushing the textdomain again
         for idx,p in ipairs(losers) do                 -- Iterate all losers and send the status
            p:send_to_inbox(_("You lost!"), 
                            _("You lost this game!"),
                            {popup=true}
                            )
         end
         winner:send_to_inbox(_("You won!"),             -- The winner get also the status
                              _("You won this game!"),
                              {popup=true}
                              )
         pop_textdomain()                                -- Reset last textdomain
      end,
   }
   pop_textdomain()                                      -- Reset textdomain from file
   return wc_definition                                  -- Return the table of the defined wincondition


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

   hooks.custom_statistic = {
      name = _("Unchanging statistic"),
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

In widelands debug builds you can open a debug console by pressing 
``Ctrl+Shift+Space``. You can enter Lua commands here that act in the global
environment: That is if you are in a scenario you can access the global
variables and alter all Lua objects that are in the global scope:

.. code-block:: lua

   print("Hello World!")
   map = wl.Game().map
   hq = map.player_slots[1].starting_field.immovable -- If this is a normal map
   hq:set_workers("barbarians_builder", 100)

This makes for excellent cheating in debug builds, but note that this is for
debug purposes only -- in network games running Lua commands this way will
desync and therefore crash the game and also replays where you changed the
game state via the debug console will not work. It is very useful
for debugging scenarios though. It is also possible to load a script from any
directory which makes testing of functions very easy. Let's assume you test
a function like:

.. code-block:: lua

   function all_players()
      for idx, player in ipairs(wl.Game().players) do
        print("Player:" player.name, player.number, player.tribe.name) 
      end
   end

Save this as ``tests.lua``. Now start a normal game, open the debug console 
by pressing ``Ctrl+Shift+Space`` and enter ``dofile("/full/path/to/tests.lua")``.
Now you can run the function ``all_players()``. If the output is not what you
expected just change the function, load the file again with ``dofile`` and 
call the function again. For convenience you can get the last 5 commands back by
pressing the Up arrow key.

Regression testing infrastructure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``test`` directory in the repository contains the regression test suite. A
test is either a savegame plus a set of Lua scripts (test_*.lua) or a map that
contains in its scripting directory a set of (test_*.lua and/or
editor_test*.lua which are only run in the Editor) files.

Each test starts Widelands using either the ``--editor``, ``--loadgame`` or
``--scenario`` switch and additionally, the ``--script`` switch can be supplied to
run a Lua script directly after the game is ready to take commands.

The tests communicate with the test runner through standard output. If a
script outputs "All Tests passed." the test is considered to pass, otherwise
to fail. Whenever a savegame is written inside a test it is later loaded by
the test runner as an additional test.

See also: `Regression Tests <https://www.widelands.org/wiki/RegressionTests/>`_

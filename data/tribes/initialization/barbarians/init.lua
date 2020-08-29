-- This file contains the basic information for the Barbarian tribe that is
-- needed before a game is loaded or the editor has been started.

dirname = path.dirname(__file__)
push_textdomain("tribes")

local r = {
   -- Basic information for the Barbarian tribe
   name = "barbarians",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"Barbarians",
   tooltip = _"The mighty tribes of the east-lands.",
   icon = dirname .. "images/icon.png",
   script = dirname .. "units.lua",

   starting_conditions = {
      dirname .. "starting_conditions/headquarters.lua";
      dirname .. "starting_conditions/fortified_village.lua";
      dirname .. "starting_conditions/trading_outpost.lua";
      dirname .. "starting_conditions/village.lua";
      dirname .. "starting_conditions/poor_hamlet.lua";
      dirname .. "starting_conditions/struggling_outpost.lua";
      dirname .. "starting_conditions/discovery.lua";
      dirname .. "starting_conditions/new_world.lua";
   }
}
pop_textdomain()
return r

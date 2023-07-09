-- This file contains the basic information for the Frisian tribe that is
-- needed before a game is loaded or the editor has been started.

local dirname = path.dirname(__file__)
push_textdomain("tribes")

local r = {
   -- Basic information for the Frisian tribe
   name = "frisians",
   author = _("The Widelands Development Team"),
   -- TRANSLATORS: This is a tribe name
   descname = _("Frisians"),
   tooltip = _("The inhabitants of the rough North Sea shore."),
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

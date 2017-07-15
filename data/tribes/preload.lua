-- RST
-- preload.lua
-- -----------
--
-- This file contains the basic information for all tribes that is
-- needed before a game is loaded or the editor has been started.

dirname = path.dirname(__file__)
set_textdomain("tribes")

return {
   -- Basic information for the Frisian tribe
   {
      name = "frisians",
      author = _"The Widelands Development Team",
      -- TRANSLATORS: This is a tribe name
      descname = _"Frisians",
      tooltip = _"The inhabitants of the rough North Sea shore.",
      icon = dirname .. "images/frisians/icon.png",

      starting_conditions = {
         dirname .. "scripting/starting_conditions/frisians/headquarters.lua";
         dirname .. "scripting/starting_conditions/frisians/fortified_village.lua";
         dirname .. "scripting/starting_conditions/frisians/trading_outpost.lua";
      }
   },

   -- Basic information for the Barbarian tribe
   {
      name = "barbarians",
      author = _"The Widelands Development Team",
      -- TRANSLATORS: This is a tribe name
      descname = _"Barbarians",
      tooltip = _"The mighty tribes of the east-lands.",
      icon = dirname .. "images/barbarians/icon.png",

      starting_conditions = {
         dirname .. "scripting/starting_conditions/barbarians/headquarters.lua";
         dirname .. "scripting/starting_conditions/barbarians/fortified_village.lua";
         dirname .. "scripting/starting_conditions/barbarians/trading_outpost.lua";
      }
   },

   -- Basic information for the Empire tribe
   {
      name = "empire",
      author = _"The Widelands Development Team",
      -- TRANSLATORS: This is a tribe name
      descname = _"Empire",
      tooltip = _"This is the culture of the Roman Empire.",
      icon = dirname .. "images/empire/icon.png",

      starting_conditions = {
         dirname .. "scripting/starting_conditions/empire/headquarters.lua";
         dirname .. "scripting/starting_conditions/empire/fortified_village.lua";
         dirname .. "scripting/starting_conditions/empire/trading_outpost.lua";
      }
   },

   -- Basic information for the Atlantean tribe
   {
      name = "atlanteans",
      author = _"The Widelands Development Team",
      -- TRANSLATORS: This is a tribe name
      descname = _"Atlanteans",
      tooltip = _"This tribe is known from the oldest tales. The sons and daughters of Atlantis.",
      icon = dirname .. "images/atlanteans/icon.png",

      starting_conditions = {
         dirname .. "scripting/starting_conditions/atlanteans/headquarters.lua";
         dirname .. "scripting/starting_conditions/atlanteans/fortified_village.lua";
         dirname .. "scripting/starting_conditions/atlanteans/trading_outpost.lua";
      }
   }
}

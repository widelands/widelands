dirname = path.dirname(__file__)

set_textdomain("tribes")

-- NOCOM(#codereview): Add a comment what this file is all about?

return {
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
      }
   }
}

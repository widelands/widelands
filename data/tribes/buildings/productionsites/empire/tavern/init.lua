push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Tavern"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "empire_inn",
      enhancement_cost = {
         planks = 2,
         marble = 2,
         marble_column = 1
      },
      enhancement_return_on_dismantle = {
         planks = 1,
         marble = 2
      }
   },

   buildcost = {
      planks = 2,
      granite = 2,
      marble = 1
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1,
      marble = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 52, 58 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 52, 58 },
      },
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 570
   },

   working_positions = {
      empire_innkeeper = 1
   },

   inputs = {
      { name = "fish", amount = 5 },
      { name = "meat", amount = 5 },
      { name = "empire_bread", amount = 5 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ration",
            "consume=empire_bread,fish,meat",
            "sleep=duration:5s",
            "playsound=sound/empire/taverns/ration priority:80%",
            "animate=working duration:18s",
            "sleep=duration:10s",
            "produce=ration"
         }
      },
   },
}

pop_textdomain()

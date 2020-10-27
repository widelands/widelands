push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      blackwood = 2,
      granite = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 58 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 41, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 41, 58 },
      },
   },

   aihints = {
      prohibited_till = 500
   },

   working_positions = {
      barbarians_baker = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "wheat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("barbarians_building", "baking pitta bread"),
         actions = {
            "return=skipped unless economy needs barbarians_bread",
            "consume=water:3 wheat:3",
            "sleep=duration:20s",
            "animate=working duration:20s",
            "produce=barbarians_bread",
            "animate=working duration:20s",
            "produce=barbarians_bread"
         }
      },
   },
}

pop_textdomain()

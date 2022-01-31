push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 4,
      ironwood = 1,
   },
   return_on_dismantle = {
      granite = 3,
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {hotspot = {43, 44}},
      idle = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 45},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
   },

   working_positions = {
      amazons_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing charcoal because ...
         descname = _("producing charcoal"),
         actions = {
            "return=skipped unless economy needs coal",
            "consume=log:6",
            "sleep=duration:30s",
            "animate=working duration:90s", -- Charcoal fires will burn for some days in real life
            "produce=coal"
         }
      },
   },
}

pop_textdomain()

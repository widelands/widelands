push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_chocolate_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Chocolate Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      balsa = 1,
      log = 2,
      rubber = 2,
      granite = 1,
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 44},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      prohibited_till = 530,
      forced_after = 720,
   },

   working_positions = {
      amazons_cook = 1
   },

   inputs = {
      { name = "cocoa_beans", amount = 4 },
      { name = "water", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing chocolate because ...
         descname = _("brewing chocolate"),
         actions = {
            "return=skipped unless economy needs chocolate",
            "consume=water:2 cocoa_beans",
            "sleep=duration:30s",
            "playsound=sound/empire/beerbubble priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=chocolate"
         },
      },
   },
}

pop_textdomain()

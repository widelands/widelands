push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Barracks"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 4,
      granite = 2,
      log = 3,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {45, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {45, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {45, 60}
      }
   },

   aihints = {
      prohibited_till = 920,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "sword_short", amount = 8 },
      { name = "fur_garment", amount = 8 },
      { name = "frisians_carrier", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("frisians_building", "recruiting soldier"),
         actions = {
            "return=skipped unless economy needs frisians_soldier",
            "consume=sword_short fur_garment frisians_carrier",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "recruit=frisians_soldier"
         }
      },
   }
}

pop_textdomain()

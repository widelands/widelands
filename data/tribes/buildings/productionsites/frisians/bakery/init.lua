push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Bakery"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "frisians_honey_bread_bakery",
      enhancement_cost = {
         brick = 3,
         log = 3,
         granite = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         brick = 2,
         granite = 1,
         log = 1,
         reed = 2
      }
   },

   buildcost = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 70},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 70},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {50, 58}
      }
   },

   aihints = {
      prohibited_till = 500,
      forced_after = 710
   },

   working_positions = {
      frisians_baker = 1
   },

   inputs = {
      { name = "barley", amount = 7 },
      { name = "water", amount = 7 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _("baking bread"),
         actions = {
            -- time total: 20.4 + 20 + 3.6 = 44 sec
            "return=skipped unless economy needs bread_frisians or workers need experience",
            "consume=water barley",
            "sleep=duration:20s400ms",
            "animate=working duration:20s",
            "produce=bread_frisians"
         },
      },
   },
}

pop_textdomain()

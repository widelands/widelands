push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_sewing_room",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Sewing Room"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "frisians_tailors_shop",
      enhancement_cost = {
         brick = 1,
         granite = 2,
         log = 2,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         granite = 1,
         log = 1
      }
   },

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
   },

   spritesheets = {
      idle = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {50, 64}
      }
   },

   aihints = {
      prohibited_till = 750,
   },

   working_positions = {
      frisians_seamstress = 1
   },

   inputs = {
      { name = "fur", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing fur garment because ...
         descname = _("sewing fur garment"),
         actions = {
            -- time total: 40 + 3.6
            "return=skipped unless economy needs fur_garment or workers need experience",
            "consume=fur:2",
            "sleep=duration:20s",
            "animate=working duration:20s",
            "produce=fur_garment"
         },
      },
   },
}

pop_textdomain()

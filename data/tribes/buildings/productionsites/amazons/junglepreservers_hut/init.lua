push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "amazons_junglepreservers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Junglepreserver's Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      rope = 1
   },
   return_on_dismantle = {
      log = 2,
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 46},
         fps = 4,
         frames = 4,
         columns = 2,
         rows = 2
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {39, 46}
      }
   },

   aihints = {
      supports_production_of = { "log", "liana" },
      space_consumer = true,
   },

   working_positions = {
      amazons_jungle_preserver = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=duration:9s"
         }
      },
   },
}

pop_textdomain()

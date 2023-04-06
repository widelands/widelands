push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_brick_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Brick Kiln"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 1,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 72},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 75},
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
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3,
   },

   working_positions = {
      frisians_brickmaker = 1
   },

   inputs = {
      { name = "granite", amount = 3 },
      { name = "clay", amount = 6 },
      { name = "coal", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start burning bricks because ...
         descname = _("burning bricks"),
         actions = {
            "return=skipped unless economy needs brick",
            "consume=granite clay:3 coal",
            "sleep=duration:30s",
            "animate=working duration:50s",
            "produce=brick:3"
         },
      },
   },
}

pop_textdomain()

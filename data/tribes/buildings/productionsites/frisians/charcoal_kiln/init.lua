push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Charcoal Kiln"),
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
      log = 1,
      reed = 1
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
      basic_amount = 1
   },

   working_positions = {
      frisians_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 },
      { name = "clay", amount = 4 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _("producing coal"),
         actions = {
            "return=skipped unless economy needs coal",
            "consume=log:3 clay",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=coal"
         },
      },
   },
}

pop_textdomain()

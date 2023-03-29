push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Weaving Mill"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring", "waterways"},

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
         hotspot = {50, 78},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 78},
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
      prohibited_till = 990
   },

   working_positions = {
      frisians_seamstress = 1
   },

   inputs = {
      { name = "fur", amount = 6 },
      { name = "reed", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving cloth because ...
         descname = _("weaving cloth"),
         actions = {
            "return=skipped unless economy needs cloth",
            "consume=fur reed",
            "sleep=duration:25s",
            "animate=working duration:20s",
            "produce=cloth"
         },
      },
   },
}

pop_textdomain()

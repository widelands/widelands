push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_dikers_house",
   descname = pgettext("frisians_building", "Diker’s House"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 2,
      log = 5,
      granite = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      reed = 1,
      log = 2,
      granite = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {75, 95},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
   },
   animations = {
      unoccupied = {
         hotspot = {75, 76}
      }
   },

   working_positions = {
      frisians_diker = 1
   },
   aihints = {},

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start diking because ...
         descname = _("diking"),
         actions = {
            "callworker=check",
            "consume=log:3",
            "callworker=dike",
            "sleep=duration:30s"
         }
      },
   },
}
pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_rope_weaver_booth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Rope Weaver Booth"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 5,
      granite = 2,
      rubber =1
   },
   return_on_dismantle = {
      log = 3,
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
      basic_amount = 1,
   },

   working_positions = {
      amazons_liana_cutter = 1
   },

   inputs = {
      { name = "liana", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving cloth because ...
         descname = _("making a rope"),
         actions = {
            "consume=liana:2",
            "sleep=duration:20s",
            "animate=working duration:35s",
            "produce=rope"
         },
      },
   },
}

pop_textdomain()

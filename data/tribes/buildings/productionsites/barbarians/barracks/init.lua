push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Barracks"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 4,
      granite = 2,
      grout = 4,
      reed = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
      grout = 1
   },

   animation_directory = dirname,
   -- TODO(Nordfriese): Make animations.
   animations = {idle = {hotspot = {55, 49}}},
   spritesheets = {
      build = {
         hotspot = {55, 49},
         frames = 3,
         columns = 1,
         rows = 3
      }
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      prohibited_till = 920
   },

   working_positions = {
      barbarians_trainer = 1
   },

   inputs = {
      { name = "ax", amount = 8 },
      { name = "barbarians_recruit", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("barbarians_building", "recruiting soldier"),
         actions = {
            "return=skipped unless economy needs barbarians_soldier",
            "consume=ax barbarians_recruit",
            "sleep=duration:30s",
            "recruit=barbarians_soldier"
         }
      },
   }
}

pop_textdomain()

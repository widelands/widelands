push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ranger’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 3
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 45, 49 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 45, 49 }
      },
      idle = {
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 45, 49 }
      },
   },

   aihints = {
      space_consumer = true,
      basic_amount = 1,
      very_weak_ai_limit = 3,
      weak_ai_limit = 5
   },

   working_positions = {
      barbarians_ranger = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _("planting trees"),
         actions = {
            "callworker=plant",
            "sleep=duration:11s500ms"
         }
      },
   },
}

pop_textdomain()

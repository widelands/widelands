push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Scout’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animation_directory = dirname,

   spritesheets = {
      build = {
         frames = 3,
         columns = 3,
         rows = 1,
         hotspot = { 55, 57 }
      },
      idle = {
         fps = 10,
         frames = 12,
         columns = 6,
         rows = 2,
         hotspot = { 55, 57 }
      },
   },

   aihints = {},

   working_positions = {
      empire_scout = 1
   },

   inputs = {
      { name = "ration", amount = 2 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
         descname = _("scouting"),
         actions = {
            "consume=ration",
            "sleep=duration:30s",
            "callworker=scout"
         }
      },
      targeted_scouting = {
         descname = _("scouting"),
         actions = {
            "consume=ration",
            "callworker=targeted_scouting"
         }
      },
   },
}

pop_textdomain()

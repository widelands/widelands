push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_gardening_center",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Gardening Center"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 4,
      granite = 2,
      rope = 2,
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
      rope = 1,
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      idle = {
         hotspot = {43, 44},
         fps = 4,
         frames = 4,
         columns = 2,
         rows = 2
      }
   },

   aihints = {
      prohibited_till = 460
   },

   working_positions = {
      amazons_jungle_master = 1
   },

   inputs = {
      { name = "coal", amount = 3 },
      { name = "fish", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start enhancing the fertility of land because ...
         descname = _("enhancing the fertility of land"),
         actions = {
            "callworker=check",
            "consume=coal fish",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
         }
      },
      terraform = {
         -- TRANSLATORS: Completed/Skipped/Did not start enhancing the fertility of land because ...
         descname = _("enhancing the fertility of land"),
         actions = {
            -- time of worker: 11.2-36.4 sec
            -- min. time: 11.2 + 8 = 19.2 sec
            -- max. time: 36.4 + 8 = 44.4 sec
            "callworker=terraform",
            "sleep=duration:8s"
         }
      },
   },
}

pop_textdomain()

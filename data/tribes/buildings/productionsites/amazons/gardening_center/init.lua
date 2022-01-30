push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
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
      --supports_production_of = { "balsa", "rubber", "ironwood" },
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
            "callworker=terraform",
            "sleep=duration:8s"
         }
      },
   },
}

pop_textdomain()

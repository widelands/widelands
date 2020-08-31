push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   name = "amazons_tapir_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Tapir Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 5,
      rope = 2,
   },
   return_on_dismantle = {
      log = 3,
      rope = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {103, 101},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {103, 101},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {103, 74},
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      amazons_tapir_breeder = 1
   },

   inputs = {
      { name = "cassavaroot", amount = 8 },
      { name = "water", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=recruit_tapir",
         }
      },
      recruit_tapir = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding tapir because ...
         descname = pgettext("amazons_building", "breeding tapir"),
         actions = {
            "return=skipped unless economy needs amazons_tapir",
            "consume=cassavaroot water",
            "sleep=duration:15s",
            "animate=working duration:15s",
            "recruit=amazons_tapir"
         }
      },
   },
}

pop_textdomain()

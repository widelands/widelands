dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_chocolate_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Chocolate Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      balsa = 1,
      log = 2,
      rubber = 2,
      granite = 1,
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 92},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 92},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 69},
      },
   },

   aihints = {
      prohibited_till = 530,
      forced_after = 720,
   },

   working_positions = {
      amazons_cook = 1
   },

   inputs = {
      { name = "cocoa_beans", amount = 4 },
      { name = "water", amount = 8 },
   },
   outputs = {
      "chocolate"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing chocolate",
         actions = {
            "return=skipped unless economy needs chocolate or workers need experience",
            "consume=water:2 cocoa_beans",
            "sleep=30000",
            "playsound=sound/empire/beerbubble 180",
            "animate=working 30000",
            "produce=chocolate"
         },
      },
   },
}

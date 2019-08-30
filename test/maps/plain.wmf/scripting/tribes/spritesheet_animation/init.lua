dirname = "test/maps/plain.wmf/" .. path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_spritesheet_animation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Spritesheet Animation"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      granite = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animations = {
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 52, 64 },
      }
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 52, 64 }
      },
      build = {
         directory = dirname,
         basename = "build",
         frames = 4,
         columns = 2,
         rows = 2,
         hotspot = { 52, 64 }
      },
      working = {
         directory = dirname,
         basename = "working",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 52, 64 }
      }
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_file_animation = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },
   outputs = {
      "blackwood"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "callworker=show_idle",
            "animate=working 17000",
            "sleep=3000"
         }
      },
   },
}

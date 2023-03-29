dirname = "test/maps/plain.wmf/" .. path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_legacy_spritesheet_animation_dir",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Spritesheet Animation"),
   icon = dirname .. "../spritesheet_animation/" .. "menu.png",
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
         directory = dirname .. "../spritesheet_animation",
         hotspot = { 52, 64 },
      }
   },

   spritesheets = {
      idle = {
         directory = dirname .. "../spritesheet_animation",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 52, 64 }
      },
      build = {
         directory = dirname .. "../spritesheet_animation",
         frames = 4,
         columns = 2,
         rows = 2,
         hotspot = { 52, 64 }
      },
      working = {
         directory = dirname .. "../spritesheet_animation",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 52, 64 },
         -- These won't be played in the test suite, just testing the table structure.
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 64
         },
         representative_frame = 3
      }
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_legacy_file_animation_dir = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "callworker=show_idle",
            "animate=working duration:17s",
            "sleep=duration:3s"
         }
      },
   },
}

dirname = "test/maps/plain.wmf/" .. path.dirname(__file__)

wl.Descriptions():new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_legacy_file_animation_dir",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "File Animation"),
   icon = dirname .. "../file_animation/" .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = {
      idle = {
         directory = dirname .. "../file_animation",
         hotspot = { 14, 21 },
         fps = 5,
         -- These won't be played in the test suite, just testing the table structure.
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 64
         },
      },
      walk = {
         directory = dirname .. "../file_animation",
         hotspot = { 9, 19 },
         fps = 10,
         directional = true
      },
      walkload = {
         directory = dirname .. "../file_animation",
         basename = "walk",
         hotspot = { 9, 19 },
         fps = 10,
         directional = true
      },
   },

   programs = {
      show_idle = {
         "findspace=size:any radius:5",
         "walk=coords",
         "animate=idle duration:10s",
         "return"
      }
   },
}

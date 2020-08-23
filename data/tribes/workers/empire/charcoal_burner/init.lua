dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Charcoal Burner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 11, 23 }
      },
      walk = {
         hotspot = { 21, 28 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 17, 27 },
         fps = 10,
         directional = true
      }
   }
}

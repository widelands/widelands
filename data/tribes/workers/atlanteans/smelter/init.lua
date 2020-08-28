dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Smelter"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      fire_tongs = 1
   },

   animations = {
      idle = {
         hotspot = { 10, 22 }
      },
      walk = {
         hotspot = { 12, 22 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 12, 24 },
         fps = 10,
         directional = true
      }
   }
}

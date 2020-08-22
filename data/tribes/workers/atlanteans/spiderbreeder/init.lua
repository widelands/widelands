dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_spiderbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Spider Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      milking_tongs = 1
   },

   animations = {
      idle = {
         hotspot = { 8, 22 }
      },
      walk = {
         hotspot = { 10, 22 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 10, 22 },
         fps = 10,
         directional = true
      }
   }
}

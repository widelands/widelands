dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_innkeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Innkeeper"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      kitchen_tools = 1
   },

   animations = {
      idle = {
         hotspot = { 4, 26 },
      },
      walk = {
         hotspot = { 6, 26 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 6, 26 },
         fps = 10,
         directional = true
      }
   }
}

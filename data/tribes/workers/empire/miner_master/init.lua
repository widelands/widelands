dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_miner_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Master Miner"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      idle = {
         hotspot = { 7, 22 }
      },
      walk = {
         hotspot = { 11, 22 },
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

push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_brewer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Brewer"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 8, 24 }
      },
      walk = {
         hotspot = { 14, 24 },
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

pop_textdomain()

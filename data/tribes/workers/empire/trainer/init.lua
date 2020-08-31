push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      armor_helmet = 1,
      spear_wooden = 1
   },

   animations = {
      idle = {
         hotspot = { 6, 23 }
      },
      walk = {
         hotspot = { 9, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 9, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_vintner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Vintner"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 12, 24 }
      },
      walk = {
         hotspot = { 12, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         -- TODO(GunChleoc): Needs a walkload animation
         basename = "walk",
         hotspot = { 12, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

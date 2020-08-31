push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_carrier_type {
   name = "empire_donkey",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Donkey"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot =  { -2, 8 },

   animations = {
      idle = {
         hotspot = { 14, 20 },
         fps = 10
      },
      walk = {
         hotspot = { 14, 25 },
         fps = 10,
         directional = true
      },
      walkload = {
         -- TODO(GunChleoc): Make animation
         basename = "walk",
         hotspot = { 14, 25 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

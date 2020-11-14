push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_carrier_type {
   name = "atlanteans_horse",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Horse"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = {-2, 12},

   animations = {
      idle = {
         hotspot = { 18, 23 },
         fps = 10
      },
      walk = {
         hotspot = { 19, 33 },
         fps = 10,
         directional = true
      },
      walkload = {
         -- TODO(GunChleoc): Make animation
         basename = "walk",
         hotspot = { 19, 33 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

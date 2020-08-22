dirname = path.dirname(__file__)

tribes:new_carrier_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_horse",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Horse"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = {-2, 12},

   aihints = {
      preciousness = {
         atlanteans = 2
      },
   },

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

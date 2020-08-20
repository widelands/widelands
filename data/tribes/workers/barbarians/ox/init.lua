dirname = path.dirname(__file__)

tribes:new_carrier_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ox",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ox"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = { -2, 13 },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 10, 25 },
         fps = 20
      },
      -- TODO(GunChleoc): Make real oxen animations
      walk = {
         hotspot = { 21, 31 },
         fps = 20,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 21, 31 },
         fps = 10,
         directional = true
      }
   },

   aihints = {
      preciousness = {
         barbarians = 2
      },
   }
}

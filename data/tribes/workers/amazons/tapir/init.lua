dirname = path.dirname (__file__)


tribes:new_carrier_type {
   msgctxt = "amazons_worker",
   name = "amazons_tapir",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Tapir"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,

   animation_directory = dirname,
   ware_hotspot = { 0, 18 },
   spritesheets = {
      walkload = {
         basename = "walk",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {15, 31}
      },
      walk = {
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {15, 31}
      },
      idle = {
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {22, 15}
      },
   },

   aihints = {
      preciousness = {
         amazons = 2
      },
   }
}

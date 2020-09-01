push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_carrier_type {
   name = "barbarians_ox",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ox"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = { -2, 13 },

   animations = {
      idle = {
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
   }
}

pop_textdomain()

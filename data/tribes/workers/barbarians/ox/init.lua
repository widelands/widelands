push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_carrier_type {
   name = "barbarians_ox",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ox"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = { -2, 13 },

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 25 }
      },
      -- TODO(hessenfarmer): Make real oxen animations
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 21, 31 }
      },
      walkload = {
         basename = "walk",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 21, 31 }
      },
   },
}

pop_textdomain()

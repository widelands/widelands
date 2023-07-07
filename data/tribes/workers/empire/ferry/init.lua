push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_ferry_type {
   name = "empire_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("empire_worker", "Ferry"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   -- TODO(Nordfriese): Make animations
   spritesheets = {
      idle = {
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      },
      walk = {
         basename = "sail",
         directional = true,
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      },
      walkload = {
         basename = "sail",
         directional = true,
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      }
   }
}

pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_ferry_type {
   name = "atlanteans_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("atlanteans_worker", "Ferry"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   -- TODO(Nordfriese): Make animations
   spritesheets = {
      idle = {
         hotspot = {22, 22},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 39,
      },
      walk = {
         basename = "sail",
         directional = true,
         hotspot = {22, 22},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      },
      walkload = {
         basename = "sail",
         directional = true,
         hotspot = {22, 22},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      }
   }
}

pop_textdomain()

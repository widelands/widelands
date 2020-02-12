dirname = path.dirname (__file__)

tribes:new_ferry_type {
   msgctxt = "empire_worker",
   name = "empire_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("empire_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   -- TODO(Nordfriese): Make animations
   spritesheets = {
      idle = {
         basename = "idle",
         directory = dirname,
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      },
      walk = {
         basename = "sail",
         directory = dirname,
         directional = true,
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      },
      walkload = {
         basename = "sail",
         directory = dirname,
         directional = true,
         hotspot = {29, 25},
         fps = 10,
         columns = 5,
         rows = 8,
         frames = 40,
      }
   }
}

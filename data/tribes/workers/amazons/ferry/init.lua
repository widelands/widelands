dirname = path.dirname (__file__)

tribes:new_ferry_type {
   msgctxt = "amazons_worker",
   name = "amazons_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   ware_hotspot = {7, -5},
   spritesheets = {
      idle = {
         hotspot = {28, 21},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walk = {
         directional = true,
         hotspot = {38, 25},
         fps = 15,
         frames = 30,
         columns = 5,
         rows = 6
      },
   },
}

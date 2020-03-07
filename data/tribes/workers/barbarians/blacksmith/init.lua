dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_blacksmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Blacksmith"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   experience = 12,
   becomes = "barbarians_blacksmith_master",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 6, 28 },
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 24 }
      }
      -- TODO(GUhnChleoc): Needs walkload animation
   }
}

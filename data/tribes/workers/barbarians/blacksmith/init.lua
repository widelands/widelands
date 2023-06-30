push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_blacksmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Blacksmith"),
   animation_directory = dirname,
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
         hotspot = { 6, 28 },
      }
   },
   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 24 }
      }
      -- TODO(GunChleoc): Needs walkload animation
   }
}

pop_textdomain()

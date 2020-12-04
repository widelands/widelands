push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_smith_basic",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Basic Smith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 16,
   becomes = "europeans_smith_normal",

   buildcost = {
      europeans_carrier = 1,
      hammer = 1,
      fire_tongs = 1,
      saw = 1
   },

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

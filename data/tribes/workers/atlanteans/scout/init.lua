dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Scout"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      atlanteans_carrier = 1
   },

   programs = {
      scout = {
         "scout=radius:15 duration:1m15s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 4, 22 }
      },
      walk = {
         hotspot = { 9, 25 },
         fps = 10,
         directional = true
      }
   }
}

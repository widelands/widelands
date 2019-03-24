dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {11, 23}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {12, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      scout = {
         "scout=15 75000", -- radius 15, 75 seconds until return
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}

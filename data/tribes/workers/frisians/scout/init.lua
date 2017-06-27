dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 23 },
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 25}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {9, 25}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      scout = {
         "scout 15 75000", -- radius 15, 75 seconds until return
         "return"
      }
   },

   animations = animations,
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 22 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {9, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      empire_carrier = 1
   },

   programs = {
      scout = {
         "scout=radius:15 duration:1m15s",
         "return"
      }
   },

   animations = animations,
}

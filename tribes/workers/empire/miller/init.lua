dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 5, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_miller",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Miller"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}

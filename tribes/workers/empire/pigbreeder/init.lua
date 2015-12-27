dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 16, 29 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {27, 29}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {27, 29}, 20)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_pigbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Pig Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}

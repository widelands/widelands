dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 8, 26 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {15, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_brewer_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Master Brewer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}

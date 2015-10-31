dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 3, 21 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {42, 30}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {42, 30}, 20)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_cattlebreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Cattle Breeder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   animations = animations,
}

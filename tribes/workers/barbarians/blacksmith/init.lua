dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 6, 28 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 24}, 15)
add_worker_animations(animations, "walkload", dirname, "walk", {11, 24}, 15)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_blacksmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Blacksmith"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   experience = 24,
   becomes = "barbarians_blacksmith_master",

   animations = animations,
}

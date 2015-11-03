dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 7, 21 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {18, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 27}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Smelter"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      fire_tongs = 1
   },

   animations = animations,
}

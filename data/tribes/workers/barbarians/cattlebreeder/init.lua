dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 3, 21 },
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {42, 30}, 20)
add_walking_animations(animations, "walkload", dirname, "walk", {42, 30}, 20)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_cattlebreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Cattle Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   animations = animations,
}

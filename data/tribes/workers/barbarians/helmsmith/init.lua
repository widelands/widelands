dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 21 }
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 21}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {7, 21}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_helmsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Helmsmith"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   animations = animations,
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 5, 26 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 27}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {14, 27}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Baker"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      bread_paddle = 1
   },

   animations = animations,
}

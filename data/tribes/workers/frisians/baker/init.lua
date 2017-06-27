dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 3, 24 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 25}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {6, 23}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Baker"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      bread_paddle = 1
   },

   experience = 23,
   becomes = "frisians_baker_master",

   animations = animations,
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 24 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {14, 24}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_brewer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Brewer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}

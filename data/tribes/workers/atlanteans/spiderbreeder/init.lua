dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 22 }
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_spiderbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Spider Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      milking_tongs = 1
   },

   animations = animations,
}

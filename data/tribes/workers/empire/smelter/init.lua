dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 23 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {18, 25}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {8, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Smelter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      fire_tongs = 1
   },

   animations = animations,
}

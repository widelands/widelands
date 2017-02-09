dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 3, 23 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 24}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {7, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Miner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      pick = 1
   },

   experience = 19,
   becomes = "empire_miner_master",

   animations = animations,
}

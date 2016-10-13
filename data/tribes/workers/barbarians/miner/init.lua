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
   msgctxt = "barbarians_worker",
   name = "barbarians_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Miner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      pick = 1
   },

   experience = 19,
   becomes = "barbarians_miner_chief",

   animations = animations,
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 3, 24 }
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 25}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {6, 23}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_miner_chief",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Chief Miner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 28,
   becomes = "barbarians_miner_master",

   animations = animations,
}

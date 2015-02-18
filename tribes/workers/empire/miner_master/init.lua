dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 7, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   name = "empire_miner_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Master Miner",
   icon = dirname .. "menu.png",
   vision_range = 2,

	-- TRANSLATORS: Helptext for a worker: Master Miner
   helptext = _"Works deep in the mines to obtain coal, iron, gold or marble.",
   animations = animations,
}

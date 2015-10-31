dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 7, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_miner_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Master Miner"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}
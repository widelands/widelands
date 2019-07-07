dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 7, 22 }
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_miner_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Master Miner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}

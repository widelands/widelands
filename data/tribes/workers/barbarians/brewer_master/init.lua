dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 26 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {15, 26}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_brewer_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Master Brewer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}

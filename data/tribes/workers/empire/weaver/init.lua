dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 24 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {6, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {6, 26}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_weaver",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Weaver"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}

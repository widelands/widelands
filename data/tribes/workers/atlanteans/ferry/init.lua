dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {20, 36},
      fps = 10
   },
   sinking = {
      pictures = path.list_files (dirname .. "sinking_??.png"),
      hotspot = {22, 35},
      fps = 7
   }
}
add_walking_animations (animations, "walk", dirname, "sail", {37, 38}, 10)
add_walking_animations (animations, "walkload", dirname, "sail", {37, 38}, 10)

tribes:new_ferry_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("atlanteans_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = {0, 20},
   animations = animations,
}

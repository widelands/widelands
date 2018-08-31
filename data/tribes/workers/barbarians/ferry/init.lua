dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {29, 19},
      fps = 10,
      scale = 4
   }
}
add_walking_animations (animations, "walk", dirname, "sail", {29, 19}, 10, 4)
add_walking_animations (animations, "walkload", dirname, "sail", {29, 19}, 10, 4)

tribes:new_ferry_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("barbarians_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}

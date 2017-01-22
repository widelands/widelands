dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 36 },
      fps = 5
   },
}
add_worker_animations(animations, "walk", dirname, "walk", {20, 34}, 10)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_recruit",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Recruit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}

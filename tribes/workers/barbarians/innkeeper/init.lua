dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 9, 24 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 24}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_innkeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Innkeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      kitchen_tools = 1
   },

   animations = animations,
}

dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {16, 30}, 5)
add_directional_animation(animations, "walk", dirname, "walk", {16, 30}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_recruit",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Recruit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}

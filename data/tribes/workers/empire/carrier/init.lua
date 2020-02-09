dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {7, 22}, 10)
add_directional_animation(animations, "walk", dirname, "walk", {7, 20}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {8, 22}, 10)


tribes:new_carrier_type {
   msgctxt = "empire_worker",
   name = "empire_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}

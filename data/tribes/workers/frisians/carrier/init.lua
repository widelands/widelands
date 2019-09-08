dirname = path.dirname (__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {17, 18}, 10)
add_directional_animation(animations, "walk", dirname, "walk", {10, 23}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_carrier_type {
   msgctxt = "frisians_worker",
   name = "frisians_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   ware_hotspot = {0, 20},
   animations = animations,
}

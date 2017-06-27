dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 14, 21 },
      fps = 5
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 19}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {7, 22}, 10)


tribes:new_carrier_type {
   msgctxt = "frisians_worker",
   name = "frisians_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}

dirname = path.dirname (__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {17, 18}, 10)
add_animation(animations, "fetch_water", dirname, "idle", {17, 18}, 10)
add_directional_animation(animations, "walk", dirname, "walk", {10, 23}, 15)

tribes:new_carrier_type {
   msgctxt = "amazons_worker",
   name = "amazons_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   programs = {
      fetch_water = {
         "findspace=size:swim radius:8",
         "walk=coords",
         "animate=fetch_water 1000",
         "createware=water",
         "return"
      }
   },

   ware_hotspot = {-1, 25},
   animations = animations,
   spritesheets = {
      walkload = {
         directory = dirname,
         basename = "walkload",
         directional = true,
         hotspot = {21, 27},
         fps = 10,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}

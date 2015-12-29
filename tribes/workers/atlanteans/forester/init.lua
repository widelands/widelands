dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 23 },
   },
   dig = {
      pictures = path.list_files(dirname .. "dig_??.png"),
      hotspot = { 5, 23 },
      fps = 5
   },
   crop = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 17, 21 },
      fps = 10
   },
   water = {
      pictures = path.list_files(dirname .. "water_??.png"),
      hotspot = { 18, 25 },
      fps = 5
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {10, 23}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Forester"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:5 avoid:field",
         "walk coords",
         "animation dig 2000", -- Play a planting animation
         "animation crop 1000", -- Play a planting animation
         "plant attrib:tree_sapling",
         "animation water 2000",
         "return"
      }
   },

   animations = animations,
}

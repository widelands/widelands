dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 23 }
   },
   dig = {
      pictures = path.list_files(dirname .. "dig_??.png"),
      hotspot = { 12, 24 },
      fps = 5
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 18, 24 },
      fps = 10
   },
   water = {
      pictures = path.list_files(dirname .. "water_??.png"),
      hotspot = { 19, 25 },
      fps = 5
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {11, 23}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {11, 23})


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_berry_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Berry Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:4",
         "walk coords",
         "animation planting 1500",
         "plant tribe:bush_tiny",
         "animation planting 1500",
         "return"
      }
   },

   animations = animations,
}

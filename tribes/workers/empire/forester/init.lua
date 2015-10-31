dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 3, 23 }
   },
   dig = {
      template = "dig_??",
      directory = dirname,
      hotspot = { 5, 22 },
      fps = 5
   },
   crop = {
      template = "plant_??",
      directory = dirname,
      hotspot = { 18, 23 },
      fps = 10
   },
   water = {
      template = "water_??",
      directory = dirname,
      hotspot = { 18, 26 },
      fps = 5
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {10, 23}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Forester"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:5 avoid:field",
         "walk coords",
         "animation dig 2000",
         "animation crop 1000",
         "plant attrib:tree_sapling",
         "animation water 2000",
         "return"
      }
   },

   animations = animations,
}
dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 10, 23 }
   },
   planting = {
      template = "plant_??",
      directory = dirname,
      hotspot = { 14, 25 },
      fps = 10
   },
   harvesting = {
      template = "harvest_??",
      directory = dirname,
      hotspot = { 19, 24 },
      fps = 10
   },
   gathering = {
      template = "gather_??",
      directory = dirname,
      hotspot = { 10, 23 },
      fps = 5
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {18, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {18, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Farmer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:2 space",
         "walk coords",
         "animation planting 4000",
         "plant tribe:field_tiny",
         "animation planting 4000",
         "return",
      },
      harvest = {
         "findobject attrib:ripe_wheat radius:2",
         "walk object",
         "playFX sound/farm scythe 220",
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 4000",
         "createware wheat",
         "return"
      }
   },

   animations = animations,
}

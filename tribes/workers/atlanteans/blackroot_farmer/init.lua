dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 24 },
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 13, 32 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 13, 32 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files(dirname .. "gather_??.png"),
      hotspot = { 13, 32 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {13, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {13, 24}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_blackroot_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Blackroot Farmer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:2",
         "walk coords",
         "animation planting 4000",
         "plant tribe:blackrootfield_tiny",
         "animation planting 4000",
         "return"
      },
      harvest = {
         "findobject attrib:ripe_blackroot radius:2",
         "walk object",
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 2000",
         "createware blackroot",
         "return"
      }
   },

   animations = animations,
}

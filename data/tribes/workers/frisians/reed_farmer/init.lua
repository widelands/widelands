dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 21, 25 }
   },
   dig = {
      pictures = path.list_files (dirname .. "dig_??.png"),
      hotspot = { 24, 29 },
      fps = 20
   },
   planting = {
      pictures = path.list_files (dirname .. "plant_??.png"),
      hotspot = { 24, 29 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 21, 25 },
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {21, 25}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {23, 27}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_reed_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Reed Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plantreed = {
         "findspace size:any radius:1",
         "walk coords",
         "animation dig 2000",
         "animation planting 1000",
         "plant attrib:seed_reed",
         "return"
      },
      harvestreed = {
         "findobject attrib:ripe_reed radius:1",
         "walk object",
         "animation harvesting 12000",
         "object harvest",
         "animation harvesting 1",
         "createware thatch_reed",
         "return"
      },
   },

   animations = animations,
}

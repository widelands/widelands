dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 23 },
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 14, 25 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 19, 23 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files(dirname .. "gather_??.png"),
      hotspot = { 9, 23 },
      fps = 5
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {18, 23}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {16, 23}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace size:any radius:2 space",
         "walk coords",
         "animation planting 4000",
         "plant tribe:barleyfield_tiny",
         "animation planting 4000",
         "return"
      },
      harvest = {
         "findobject attrib:ripe_barley radius:2",
         "walk object",
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 4000",
         "createware barley",
         "return"
      }
   },

   animations = animations,
}

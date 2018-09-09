dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   planting = {
      pictures = path.list_files (dirname .. "plant_??.png"),
      hotspot = { 13, 25 },
      fps = 20
   },
   harvesting = {
      pictures = path.list_files (dirname .. "harvest_??.png"),
      hotspot = { 12, 25 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files (dirname .. "gather_??.png"),
      hotspot = { 12, 18 },
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {11, 24}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {16, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting 8000",
         "plant=attrib:seed_barley",
         "animate=planting 8000",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_barley radius:2",
         "walk=object",
         "animate=harvesting 10000",
         "callobject=harvest",
         "animate=gathering 8000",
         "createware=barley",
         "return"
      }
   },

   animations = animations,
}

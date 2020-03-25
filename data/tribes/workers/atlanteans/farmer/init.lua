dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 23 },
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 13, 32 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 18, 32 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files(dirname .. "gather_??.png"),
      hotspot = { 10, 34 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {18, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {18, 23}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting 6000",
         "plant=attrib:seed_corn",
         "animate=planting 6000",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_corn radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvesting 10000",
         "callobject=harvest",
         "animate=gathering 4000",
         "createware=corn",
         "return"
      }
   },

   animations = animations,
}

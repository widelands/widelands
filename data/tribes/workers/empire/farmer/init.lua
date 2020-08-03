dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 23 }
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 14, 25 },
      fps = 10
   },
   harvesting = {
      pictures = path.list_files(dirname .. "harvest_??.png"),
      hotspot = { 19, 24 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files(dirname .. "gather_??.png"),
      hotspot = { 10, 23 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {18, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {18, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting duration:6s",
         "plant=attrib:seed_wheat",
         "animate=planting duration:6s",
         "return",
      },
      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=wheat",
         "return"
      }
   },

   animations = animations,
}

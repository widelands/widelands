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
add_directional_animation(animations, "walk", dirname, "walk", {13, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {13, 24}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_blackroot_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Blackroot Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2",
         "walk=coords",
         "animate=planting duration:9s",
         "plant=attrib:seed_blackroot",
         "animate=planting duration:9s",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_blackroot radius:2",
         "walk=object",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=blackroot",
         "return"
      }
   },

   animations = animations,
}

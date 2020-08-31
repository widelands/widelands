push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_blackroot_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Blackroot Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
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

   animations = {
      idle = {
         hotspot = { 9, 24 }
      },
      planting = {
         basename = "plant",
         hotspot = { 13, 32 },
         fps = 10
      },
      harvesting = {
         basename = "harvest",
         hotspot = { 13, 32 },
         fps = 10
      },
      gathering = {
         basename = "gather",
         hotspot = { 13, 32 },
         fps = 10
      },
      walk = {
         hotspot = { 13, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 13, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Farmer"),
   animation_directory = dirname,
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
         "animate=planting duration:6s",
         "plant=attrib:seed_corn",
         "animate=planting duration:6s",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_corn radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=corn",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 10, 23 },
      },
      planting = {
         basename = "plant",
         hotspot = { 13, 32 },
         fps = 10
      },
      harvesting = {
         basename = "harvest",
         hotspot = { 18, 32 },
         fps = 10
      },
      gathering = {
         basename = "gather",
         hotspot = { 10, 34 },
         fps = 5
      },
      walk = {
         hotspot = { 18, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 18, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

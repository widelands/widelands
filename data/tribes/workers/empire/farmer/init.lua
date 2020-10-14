push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Farmer"),
   animation_directory = dirname,
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
         "playsound=sound/farm/scythe priority:80% allow_multiple",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=wheat",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 10, 23 }
      },
      planting = {
         basename = "plant",
         hotspot = { 14, 25 },
         fps = 10
      },
      harvesting = {
         basename = "harvest",
         hotspot = { 19, 24 },
         fps = 10
      },
      gathering = {
         basename = "gather",
         hotspot = { 10, 23 },
         fps = 5
      },
      walk = {
         hotspot = { 18, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 18, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
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
   },

   spritesheets = {
      planting = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 13, 32 }
      },
      harvesting = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 18, 32 }
      },
      gathering = {
         fps = 5,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 34 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 18, 23 }
      },
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 18, 23 }
      },
   },
}

pop_textdomain()

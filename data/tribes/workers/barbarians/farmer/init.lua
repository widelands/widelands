dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:6s",
         "plant=attrib:seed_wheat",
         "animate=plant duration:6s",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvest duration:10s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=wheat",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 8, 17 },
      },
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 14, 18 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 12, 19 }
      },
      plant = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 19 }
      },
      harvest = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 17, 21 }
      },
      gather = {
         fps = 5,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 19 }
      }
   }
}

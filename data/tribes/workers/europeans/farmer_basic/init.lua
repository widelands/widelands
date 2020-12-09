push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_farmer_basic",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Basic Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 16,
   becomes = "europeans_farmer_normal",

   buildcost = {
      europeans_carrier = 1,
      scythe = 1,
      basket = 1
   },

   programs = {
      plant_barley = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_barley",
         "animate=plant duration:8s",
         "return"
      },
      plant_berrybush = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_berrybush",
         "animate=plant duration:8s",
         "return"
      },
      plant_blackroot = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_blackroot",
         "animate=plant duration:8s",
         "return"
      },
      plant_corn = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_corn",
         "animate=plant duration:8s",
         "return"
      },
      plant_grape = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_grapes",
         "animate=plant duration:8s",
         "return"
      },
      plant_reed = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_reed",
         "animate=plant duration:8s",
         "return"
      },
      plant_rye = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_rye",
         "animate=plant duration:8s",
         "return"
      },
      plant_wheat = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_wheat",
         "animate=plant duration:8s",
         "return"
      },
      harvest_barley = {
         "findobject=attrib:ripe_barley radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=barley",
         "return"
      },
      harvest_berrybush = {
         "findobject=attrib:ripe_bush radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=fruit",
         "return"
      },
      harvest_blackroot = {
         "findobject=attrib:ripe_blackroot radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=blackroot",
         "return"
      },
      harvest_corn = {
         "findobject=attrib:ripe_corn radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=corn",
         "return"
      },
      harvest_grape = {
         "findobject=attrib:ripe_grapes radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=grape",
         "return"
      },
      harvest_reed = {
         "findobject=attrib:ripe_reed radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=reed",
         "return"
      },
      harvest_rye = {
         "findobject=attrib:ripe_rye radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
         "createware=rye",
         "return"
      },
      harvest_wheat = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:6s",
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

pop_textdomain()

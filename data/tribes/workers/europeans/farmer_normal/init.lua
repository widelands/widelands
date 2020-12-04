push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_farmer_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_farmer_advanced",

   programs = {
      plant_barley = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_barley",
         "animate=plant duration:8s",
         "return"
      },
      plant_berrybush = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_berrybush",
         "animate=plant duration:8s",
         "return"
      },
      plant_blackroot = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_blackroot",
         "animate=plant duration:8s",
         "return"
      },
      plant_corn = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_corn",
         "animate=plant duration:8s",
         "return"
      },
      plant_grape = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_grapes",
         "animate=plant duration:8s",
         "return"
      },
      plant_reed = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_reed",
         "animate=plant duration:8s",
         "return"
      },
      plant_rye = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_rye",
         "animate=plant duration:8s",
         "return"
      },
      plant_wheat = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=plant duration:8s",
         "plant=attrib:seed_wheat",
         "animate=plant duration:8s",
         "return"
      },
      harvest_barley = {
         "findobject=attrib:ripe_barley radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=barley",
         "return"
      },
      harvest_berrybush = {
         "findobject=attrib:ripe_bush radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=fruit",
         "return"
      },
      harvest_blackroot = {
         "findobject=attrib:ripe_blackroot radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=blackroot",
         "return"
      },
      harvest_corn = {
         "findobject=attrib:ripe_corn radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=corn",
         "return"
      },
      harvest_grape = {
         "findobject=attrib:ripe_grapes radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=grape",
         "return"
      },
      harvest_reed = {
         "findobject=attrib:ripe_reed radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=reed",
         "return"
      },
      harvest_rye = {
         "findobject=attrib:ripe_rye radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=rye",
         "return"
      },
      harvest_wheat = {
         "findobject=attrib:ripe_wheat radius:3",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:8s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=wheat",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 10, 23 },
      },
      plant = {
         basename = "plant",
         hotspot = { 13, 32 },
         fps = 10
      },
      harvest = {
         basename = "harvest",
         hotspot = { 18, 32 },
         fps = 10
      },
      gather = {
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

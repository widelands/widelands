push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-6
         -- avg. step in ideal case (free 4 nearest fields): (4*2)/4 = 2
         -- avg. step in worst case (free 4 furthest fields): (1*4+2*5+1*6)/4 = 5
         -- avg. step in std case (free all fields except road): (4*2+4*3+2*4+2*5+1*6)/13 = 3.385
         -- min. worker time: 2 * 2 * 1.8 + 6 + 6 = 19.2 sec
         -- max. worker time: 2 * 6 * 1.8 + 6 + 6 = 33.6 sec
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant duration:6s",
         "plant=attrib:seed_wheat",
         "animate=plant duration:6s",
         "return"
      },
      harvest = {
         -- min. worker time: 2 * 2 * 1.8 + 10 + 4 = 21.2 sec
         -- max. worker time: 2 * 6 * 1.8 + 10 + 4 = 35.6 sec
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
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

pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_vinefarmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Vine Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      shovel = 1,
      basket = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig duration:5s",
         "plant=attrib:seed_grapes",
         "animate=planting duration:5s",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_grapes radius:1",
         "walk=object",
         "animate=gathering duration:8s",
         "callobject=harvest",
         "animate=gathering duration:2s",
         "createware=grape",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 4, 23 }
      },
   },

   spritesheets = {
      dig = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 8, 24 }
      },
      planting = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 15, 23 }
      },
      gathering = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 9, 22 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 23 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 24 }
      },
   }
}

pop_textdomain()

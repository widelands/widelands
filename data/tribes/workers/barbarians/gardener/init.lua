push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_gardener",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gardener"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=plant duration:6s500ms",
         "plant=attrib:seed_reed",
         "animate=plant duration:6s500ms",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_reed radius:1",
         "walk=object",
         "animate=harvest duration:14s",
         "callobject=harvest",
         "createware=reed",
         "return"
      },
   },

   animations = {
      idle = {
         hotspot = { -4, 11 }
      }
   },
   spritesheets = {
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
         hotspot = { 7, 23 }
      },
      plant = {
         fps = 10,
         frames = 15,
         rows = 5,
         columns = 3,
         hotspot = { 10, 21 }
      },
      harvest = {
         fps = 5,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 22 }
      }
   }
}

pop_textdomain()

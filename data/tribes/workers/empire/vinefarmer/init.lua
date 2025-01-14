push_textdomain("tribes")

local dirname = path.dirname(__file__)

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
         -- steps from building to field: 2-4
         -- avg. step in ideal case (free 4 nearest fields): (2*2+2*3)/4 = 2.5
         -- avg. step in worst case (free 4 furthest fields): (1*2+2*3+1*4)/4 = 3
         -- avg. step in std case (free all fields): (2*2+2*3+1*4)/5 = 2.8
         -- min. worker time: 2 * 2.5 * 1.8 + 5 + 5 = 19   sec
         -- max. worker time: 2 * 3   * 1.8 + 5 + 5 = 20.8 sec
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig duration:5s",
         "plant=attrib:seed_grapes",
         "animate=planting duration:5s",
         "return"
      },
      harvest = {
         -- min. worker time: 2 * 2.5 * 1.8 + 8 + 2 = 19   sec
         -- max. worker time: 2 * 3   * 1.8 + 8 + 2 = 20.8 sec
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

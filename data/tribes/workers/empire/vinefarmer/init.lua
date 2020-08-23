dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
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
      dig = {
         hotspot = { 8, 24 },
         fps = 10
      },
      planting = {
         basename = "plant",
         hotspot = { 15, 23 },
         fps = 10
      },
      gathering = {
         basename = "gather",
         hotspot = { 9, 22 },
         fps = 10
      },
      walk = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      }
   }
}

dirname = "tribes/workers/frisians/claydigger/"

push_textdomain("scenario_fri03.wmf")

descriptions:new_worker_type {
   name = "frisians_diker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Diker"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      dike = {
         "findspace=size:swim radius:5 terraform",
         "walk=coords",
         "animate=dig duration:40s",
         "terraform",
         "animate=dig duration:40s",
         "return"
      }
   },

   ware_hotspot = {0, 20},

   animation_directory = dirname,
   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      dig = {
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
   },
   animations = {idle = {hotspot = {8, 23}}}
}

pop_textdomain()

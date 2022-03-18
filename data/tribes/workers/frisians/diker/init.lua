push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_diker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Diker"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      check = {
         "findspace=size:swim radius:5 terraform:diking",
      },
      dike = {
         "findspace=size:swim radius:5 terraform:diking",
         -- Note that while the diker CAN also terraform non-water nodes, he only works
         -- directly at the shore. This introduces some randomness in the pattern of how
         -- many water tiles actually make the two-step transformation to arable land.
         "walk=coords",
         "animate=work duration:60s",
         "terraform=diking",
         "animate=work duration:20s",
         "return"
      }
   },

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
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
      work = {
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
   },
   animations = {idle = {hotspot = {8, 26}}}
}

pop_textdomain()

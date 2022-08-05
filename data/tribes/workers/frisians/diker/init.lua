push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_diker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Diker"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      check_build = {
         "findspace=size:swim radius:5 terraform:diking",
      },
      check_dike = {
         "findobject=attrib:dike radius:5",
      },
      build = {
         "findspace=size:swim radius:5 terraform:diking",
         "walk=coords",
         "animate=work duration:70s",
         "plant=attrib:dike",
         "return"
      },
      dike = {
         "findobject=attrib:dike radius:5",
         "walk=object",
         "animate=work duration:30s",
         "callobject=maybe_remove",
         "animate=work duration:30s",
         "terraform=diking",
         "animate=work duration:30s",
         "return"
      },
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

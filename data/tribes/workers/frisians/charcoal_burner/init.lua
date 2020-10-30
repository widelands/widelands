push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_worker_type {
   name = "frisians_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Charcoal Burner"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      collect_coal = {
         "findobject=attrib:pond_coal radius:8",
         "walk=object",
         "animate=collecting duration:8s",
         "callobject=fall_dry",
         "createware=coal",
         "return"
      },
      make_stack = {
         "findobject=attrib:pond_dry radius:8",
         "walk=object",
         "animate=stacking_1 duration:14s",
         "callobject=with_stack",
         "animate=stacking_2 duration:1s",
         "return"
      },
      find_pond = {
         "findobject=attrib:pond_dry radius:8",
      }
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 23}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      collecting = {
         directory = dirname,
         basename = "collecting",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {10, 19}
      },
      stacking_1 = {
         directory = dirname,
         basename = "stacking_1",
         fps = 10,
         frames = 15,
         columns = 5,
         rows = 3,
         hotspot = {9, 19}
      },
      stacking_2 = {
         directory = dirname,
         basename = "stacking_2",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {9, 19}
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()

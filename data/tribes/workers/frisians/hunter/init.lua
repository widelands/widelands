push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Hunter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hunting_spear = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:14 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s",
         "callobject=remove",
         "createware=meat",
         "return"
      }
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 23}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 26}
      },
      idle = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {23, 22}
      },
   },
}

pop_textdomain()

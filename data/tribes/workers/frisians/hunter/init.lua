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
         -- steps from building to animal: 2-17, mean 10.819
         -- min. worker time: 2 * 2 * 1.8 + 1 = 8.2 sec
         -- max. worker time: 2 * 17 * 1.8 + 1 = 62.2 sec
         -- mean worker time: 2 * 10.819 * 1.8 + 1 = 39.948 sec
         -- chasing of the game also changes time, average should remain the same
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

push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Hunter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hunting_spear = 1
   },

   programs = {
      hunt = {
         -- steps from building to animal: 2-16, min+max average 9
         -- min. worker time: 2 * 2 * 1.8 + 1 = 8.2 sec
         -- max. worker time: 2 * 16 * 1.8 + 1 = 58.6 sec
         -- avg. worker time: 2 * 9 * 1.8 + 1 = 33.4 sec
         -- chasing of the game also changes time, average should remain the same
         "findobject=type:bob radius:13 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 20 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 26 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 26 }
      }
   }
}

pop_textdomain()

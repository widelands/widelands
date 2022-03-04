push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Hunter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hunting_bow = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:13 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s500ms",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 15,
         rows = 5,
         columns = 3,
         hotspot = { 6, 29 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 14, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 13, 23 }
      },
   },
}

pop_textdomain()

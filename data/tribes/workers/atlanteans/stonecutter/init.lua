push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_stonecutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Stonecutter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/atlanteans/cutting/stonecutter priority:50% allow_multiple",
         "animate=hacking duration:18s500ms",
         "callobject=shrink",
         "createware=granite",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 7, 20 }
      },
   },

   spritesheets = {
      hacking = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 10, 19 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 21 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 23 }
      },
   },
}

pop_textdomain()

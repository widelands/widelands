push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_stonecutter_basic",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Basic Stonecutter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 16,
   becomes = "europeans_stonecutter_normal",

   buildcost = {
      europeans_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:50% allow_multiple",
         "animate=hack duration:17s500ms",
         "callobject=shrink",
         "createware=granite",
         "return"
      },
      mine_granite = {
         "findspace=size:any radius:6 resource:resource_stones",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:50% allow_multiple",
         "animate=hack duration:17s500ms",
         "mine=resource_stones radius:1",
         "createware=granite",
         "return"
      },
   },

   animations = {
      idle = {
         hotspot = { 6, 16 },
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 17 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 19 }
      },
      hack = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 6, 17 }
      }
   }
}

pop_textdomain()

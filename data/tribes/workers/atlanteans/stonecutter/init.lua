push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
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
      hacking = {
         hotspot = { 10, 19 },
         fps = 10
      },
      walk = {
         hotspot = { 10, 21 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 10, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

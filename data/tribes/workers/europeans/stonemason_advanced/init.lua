push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_stonemason_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Stonemason"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:10",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:70% allow_multiple",
         "animate=hacking duration:12s500ms",
         "callobject=shrink",
         "createware=granite",
         "return"
      },
      cut_marble = {
         "findobject=attrib:rocks radius:10",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter priority:70% allow_multiple",
         "animate=hacking duration:12s500ms",
         "callobject=shrink",
         "createware=marble",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 9, 23 },
         fps = 10
      },
      hacking = {
         hotspot = { 8, 23 },
         fps = 10
      },
      walk = {
         hotspot = { 9, 22 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 25 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

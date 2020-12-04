push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_hunter_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Hunter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_hunter_advanced",

   programs = {
      hunt = {
         "findobject=type:bob radius:14 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s500ms",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   animations = {
      idle = {
         basename = "shooting",
         hotspot = { 6, 29 },
         fps = 10
      },
      walk = {
         hotspot = { 14, 22 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 13, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()

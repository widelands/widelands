dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
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

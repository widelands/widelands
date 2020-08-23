dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hunting_spear = 1
   },
   programs = {
      hunt = {
         "findobject=type:bob radius:13 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 10, 21 },
         fps = 10
      },
      walk = {
         hotspot = { 9, 32 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 9, 32 },
         fps = 10,
         directional = true
      }
   }
}

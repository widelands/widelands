dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {23, 22},
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {10, 23}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {11, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hunting_spear = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:14 attrib:eatable",
         "walk=object",
         "animate=idle 1000",
         "callobject=remove",
         "createware=meat",
         "return"
      }
   },

   animations = animations,
}

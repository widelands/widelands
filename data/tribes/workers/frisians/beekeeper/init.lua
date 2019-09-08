dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   beeswarm = {
      pictures = path.list_files (dirname .. "beeswarm_??.png"),
      hotspot = { 26, 31 },
      fps = 50
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_beekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Beekeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      bees = {
         "findobject=attrib:flowering radius:5",
         "walk=object",
         "animate=beeswarm 10000",
         "createware=honey",
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}

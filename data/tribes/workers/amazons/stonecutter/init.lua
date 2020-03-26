dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   hacking = {
      pictures = path.list_files (dirname .. "hacking_??.png"),
      hotspot = { 17, 31 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_stonecutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Stonecutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "animate=hacking 35000",
         "callobject=shrink",
         "createware=granite",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 20},
   animations = animations,
}

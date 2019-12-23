dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {9, 23},
      fps = 10
   },
   hacking = {
      pictures = path.list_files (dirname .. "hacking_??.png"),
      hotspot = {19, 33},
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {16, 23}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {12, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      felling_ax = 1
   },
   
   experience = 15,
   becomes = "amazons_woodcutter_master",

   programs = {
      harvest = {
         "findobject=attrib:normal_tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 30000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=log",
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}

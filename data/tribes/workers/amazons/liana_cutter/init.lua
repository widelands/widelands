dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   work = {
      pictures = path.list_files (dirname .. "work_??.png"),
      hotspot = { 15, 20 },
      fps = 20
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_liana_cutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Liana Cutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      machete = 1
   },

   programs = {
      cut = {
         "findobject=attrib:tree radius:6",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=work 4000",
         "playsound=sound/woodcutting/tree_falling 130",
         "animate=idle 1000",
         "createware=liana",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 20},
   animations = animations,
}

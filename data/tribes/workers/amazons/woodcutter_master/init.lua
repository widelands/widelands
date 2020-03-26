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
   name = "amazons_woodcutter_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Master Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      harvest_ironwood = {
         "findobject=attrib:tree_ironwood radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=ironwood",
         "return"
      },
      harvest_rubber = {
         "findobject=attrib:tree_rubber radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=rubber",
         "return"
      },
      harvest_balsa = {
         "findobject=attrib:tree_balsa radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=balsa",
         "return"
      },
      harvest = {
         "findobject=attrib:normal_tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=idle 2000",
         "createware=log",
         "return"
      },
   },

   animation_directory = dirname,
   ware_hotspot = {0, 20},
   animations = animations,
}

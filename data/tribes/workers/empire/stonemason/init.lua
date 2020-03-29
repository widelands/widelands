dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 23 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 8, 23 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {8, 25}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Stonemason"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter 220",
         "animate=hacking 17500",
         "callobject=shrink",
         "createware=granite",
         "return"
      },
      cut_marble = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter 220",
         "animate=hacking 17500",
         "callobject=shrink",
         "createware=marble",
         "return"
      }
   },

   animations = animations,
}

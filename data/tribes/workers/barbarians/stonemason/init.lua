dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 21 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 8, 21 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {7, 22}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Stonemason"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "playsound=sound/stonecutting/stonecutter 192",
         "animate=hacking 17500",
         "callobject=shrink",
         "createware=granite",
         "return"
      }
   },

   animations = animations,
}

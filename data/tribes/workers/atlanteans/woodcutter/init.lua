dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 22 }
   },
   sawing = {
      pictures = path.list_files(dirname .. "sawing_??.png"),
      hotspot = { 22, 19 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {16, 31}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {13, 29}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      saw = 1
   },

   programs = {
      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/atlanteans/saw/sawing priority:180%",
         "animate=sawing duration:20s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=log",
         "return"
      }
   },

   animations = animations,
}

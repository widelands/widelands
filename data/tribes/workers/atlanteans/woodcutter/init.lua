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
add_walking_animations(animations, "walk", dirname, "walk", {16, 31}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {13, 29}, 10)


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
         "findobject attrib:tree radius:10",
         "walk object",
         "play_sound sound/atlanteans/saw sawing 230",
         "animation sawing 10000",
         "play_sound sound/woodcutting tree-falling 130",
         "object fall",
         "animation idle 2000",
         "createware log",
         "return"
      }
   },

   animations = animations,
}

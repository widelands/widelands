dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 5, 23 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 19, 17 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {10, 22}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {10, 21}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      felling_ax = 1
   },

   programs = {
      chop = {
         "findobject attrib:tree radius:10",
         "walk object",
         "animation hacking 10000",
         "object fall",
         "animation idle 2000",
         "createware log",
         "return"
      }
   },

   animations = animations,
}

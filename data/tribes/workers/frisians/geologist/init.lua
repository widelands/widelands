dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = { 21, 25 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files (dirname .. "hacking_??.png"),
      hotspot = { 21, 25 },
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {21, 25}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Geologist"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   programs = {
      expedition = {
         "geologist 15 5 search"
      },
      search = {
         "animation hacking 3000",
         "animation idle 1000",
         "animation hacking 2000",
         "animation idle 1000",
         "animation hacking 3000",
         "geologist_find"
      }
   },

   animations = animations,
}

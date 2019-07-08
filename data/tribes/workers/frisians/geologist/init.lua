dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
      fps = 10
   },
   hacking = {
      pictures = path.list_files (dirname .. "hacking_??.png"),
      hotspot = { 9, 17 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 23}, 15)

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
         "repeatsearch=15 5 search"
      },
      search = {
         "animate=hacking 3000",
         "animate=idle 1000",
         "animate=hacking 2000",
         "animate=idle 1000",
         "animate=hacking 3000",
         "findresources"
      }
   },

   animations = animations,
}

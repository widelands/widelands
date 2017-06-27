dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 21 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 11, 18 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 21}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {9, 21}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Geologist"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   programs = {
      -- Expedition is the main program
      -- The specialized geologist command walks the geologist around his starting
      -- location, executing the search program from time to time.
      expedition = {
         "geologist 15 5 search"
      },
      -- Search program, executed when we have found a place to hack on
      search = {
         "animation hacking 5000",
         "animation idle 2000",
         "animation hacking 3000",
         "geologist_find"
      }
   },

   animations = animations,
}

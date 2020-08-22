dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Geologist"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   programs = {
      -- Expedition is the main program
      -- The specialized geologist command walks the geologist around his starting
      -- location, executing the search program from time to time.
      expedition = {
         "repeatsearch=search repetitions:15 radius:5"
      },
      -- Search program, executed when we have found a place to hack on
      search = {
         "animate=hacking duration:5s",
         "animate=idle duration:2s",
         "playsound=sound/hammering/geologist_hammer priority:50% allow_multiple",
         "animate=hacking duration:3s",
         "findresources"
      }
   },

   animations = {
      idle = {
         hotspot = { 10, 23 },
         fps = 10
      },
      hacking = {
         hotspot = { 12, 20 },
         fps = 10
      },
      walk = {
         hotspot = { 11, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 11, 23 },
         fps = 10,
         directional = true
      }
   }
}

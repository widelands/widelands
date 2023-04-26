push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Geologist"),
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

   spritesheets = {
      idle = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 10, 23 }
      },
      hacking = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 12, 20 }
      },
      walk = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 23 }
      },
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 23 }
      },
   },
}

pop_textdomain()

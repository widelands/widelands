push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_carrier_type {
   name = "empire_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carrier"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   spritesheets = {
      idle = {
         fps = 10,
         frames = 95,
         rows = 11,
         columns = 9,
         hotspot = { 7, 22 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 20 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 21 }
      },
   },
}

pop_textdomain()

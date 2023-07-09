push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_carrier_type {
   name = "barbarians_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Carrier"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   spritesheets = {
      idle = {
         fps = 5,
         frames = 100,
         columns = 10,
         rows = 10,
         hotspot = { 11, 18 }
      },
      walk = {
         fps = 10,
         frames = 10,
         columns = 3,
         rows = 4,
         directional = true,
         hotspot = { 6, 16 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         columns = 3,
         rows = 4,
         directional = true,
         hotspot = { 6, 18 }
      }
   }
}

pop_textdomain()

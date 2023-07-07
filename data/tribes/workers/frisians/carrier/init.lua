push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_carrier_type {
   name = "frisians_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Carrier"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 23}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      idle = {
         fps = 10,
         frames = 124,
         columns = 14,
         rows = 9,
         hotspot = {17, 18}
      },
   },
}

pop_textdomain()

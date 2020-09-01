push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_carrier_type {
   name = "empire_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = {
      idle = {
         hotspot = { 7, 22 },
         fps = 10
      },
      walk = {
         hotspot = { 7, 20 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 22 },
         fps = 10,
         directional = true
      }
   },
}

pop_textdomain()

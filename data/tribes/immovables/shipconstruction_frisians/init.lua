dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "shipconstruction" },
   programs = {
      program = {
         "construct=idle 5000 210000",
         "transform=bob frisians_ship",
      }
   },
   buildcost = {
      log = 10,
      clay = 2,
      cloth = 6
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "build",
         hotspot = {75, 141},
         frames = 6,
         columns = 3,
         rows = 2,
         fps = 1,
         representative_frame = 3
      },
   }
}

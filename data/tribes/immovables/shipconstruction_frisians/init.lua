dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "frisians_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   representative_image = dirname .. "build_03.png",
   attributes = { "shipconstruction" },
   programs = {
      program = {
         "construction=idle 5000 210000",
         "transform=bob tribe:frisians_ship",
      }
   },
   buildcost = {
      log = 10,
      clay = 2,
      cloth = 6
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "build_??.png"),
         hotspot = { 192, 226 },
         fps = 1
      },
   }
}

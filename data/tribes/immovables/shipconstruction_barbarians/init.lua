dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   size = "small",
   programs = {
      program = {
         "construction=idle 5000 210000",
         "transform=bob tribe:barbarians_ship",
      }
   },
   buildcost = {
      blackwood = 10,
      log = 2,
      cloth = 4
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 115, 82 },
         fps = 1
      },
   }
}

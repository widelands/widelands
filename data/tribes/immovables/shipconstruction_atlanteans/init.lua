dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "atlanteans_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   attributes = { "shipconstruction" },
   programs = {
      program = {
         "construct=idle 5000 210000",
         "transform=bob atlanteans_ship",
      }
   },
   buildcost = {
      planks = 10,
      log = 2,
      spidercloth = 4
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 118, 94 },
         fps = 1,
         representative_frame = 5
      },
   }
}

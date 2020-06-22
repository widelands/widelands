dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "amazons_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "shipconstruction" },
   programs = {
      program = {
         "construct=idle 5000 210000",
         "transform=bob amazons_ship",
      }
   },
   buildcost = {
      log = 5,
      balsa = 5,
      rubber = 3,
      rope = 3
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {73, 37},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cassavarootfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassavaroot Field (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   attributes = { "field", "seed_cassava" },
   programs = {
      program = {
         "animate=idle 30000",
         "transform=cassavarootfield_small",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {3, 7},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

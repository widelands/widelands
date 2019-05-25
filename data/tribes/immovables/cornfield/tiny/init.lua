dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cornfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "field", "seed_corn" },
   programs = {
      program = {
         "animate=idle 30000",
         "transform=cornfield_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 35, 34 },
      },
   }
}

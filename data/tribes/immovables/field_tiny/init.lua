dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "field_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Field (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "field", "seed_wheat" },

   programs = {
      program = {
         "animate=idle 30000",
         "transform=field_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 32, 21 },
      },
   }
}

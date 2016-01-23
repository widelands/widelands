dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "reed_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed (tiny)"),
   size = "small",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 22000",
         "transform=reed_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 12, 8 },
      },
   }
}

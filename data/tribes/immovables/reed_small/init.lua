dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "reed_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed (small)"),
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 28000",
         "transform=reed_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 13, 12 },
      },
   }
}

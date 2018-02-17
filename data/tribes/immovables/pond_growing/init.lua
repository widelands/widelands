dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_growing",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Pond With Fish Spawn"),
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   programs = {
      program = {
         "animate=idle 140000",
         "transform=pond_mature",
      },
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = { 8, 5 },
         scale = 2.5
      },
   }
}

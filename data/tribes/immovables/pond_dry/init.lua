dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_dry",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Dry Pond"),
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "pond_dry" },
   programs = {
      program = {
         "animate=idle 400000",
         "remove=",
      },
      with_fish = {
         "transform=pond_growing",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = { 8, 5 },
         scale = 2.5
      },
   }
}

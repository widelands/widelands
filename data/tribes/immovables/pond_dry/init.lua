dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_dry",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Dry Pond"),
   size = "small",
   attributes = { "pond_dry", "pond" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove=",
      },
      with_fish = {
         "transform=pond_growing",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 12, 8 },
      },
   }
}

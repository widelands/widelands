dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_mature",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Pond with fish"),
   size = "small",
   attributes = { "pond_mature", "pond" },
   programs = {
      program = {
         "animate=idle 300000",
         "transform=pond_growing",
      },
      fall_dry = {
         "transform=pond_dry",
      },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 12, 8 },
      },
   }
}

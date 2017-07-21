dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_growing",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Pond with young fish"),
   size = "small",
   attributes = { "pond_growing", "pond" },
   programs = {
      program = {
         "animate=idle 60000",
         "transform=pond_mature",
      },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 12, 8 },
      },
   }
}

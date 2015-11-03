dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cornfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (ripe)"),
   size = "small",
   attributes = { "ripe_corn", "field" },
   programs = {
      program = {
         "animate=idle 50000",
         "remove=",
      },
      harvest = {
         "transform=cornfield_harvested",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 31, 41 },
      },
   }
}

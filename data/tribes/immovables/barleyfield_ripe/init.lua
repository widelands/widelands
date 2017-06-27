dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (ripe)"),
   size = "small",
   attributes = { "ripe_barley", "field" },
   programs = {
      program = {
         "animate=idle 1500000",
         "remove=",
      },
      harvest = {
         "transform=barleyfield_harvested"
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 18 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "field_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Field (medium)"),
   size = "small",
   attributes = { "field", "flowering" },
   programs = {
      program = {
         "animate=idle 50000",
         "transform=field_ripe",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 12 },
      },
   }
}

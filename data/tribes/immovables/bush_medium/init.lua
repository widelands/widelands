dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "bush_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Bush (medium)"),
   size = "small",
   attributes = { "bush", "flowering" },
   programs = {
      program = {
         "animate=idle 40000",
         "transform=bush_ripe",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 13, 14 },
      },
   }
}

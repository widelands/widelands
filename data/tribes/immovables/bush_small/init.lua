dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "bush_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Bush (small)"),
   size = "small",
   attributes = { "bush" },
   programs = {
      program = {
         "animate=idle 28000",
         "transform=bush_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 13, 12 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "grapevine_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (small)"),
   size = "medium",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 28000",
         "transform=grapevine_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 18 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "grapevine_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (medium)"),
   size = "medium",
   attributes = { "field", "flowering" },
   programs = {
      program = {
         "animate=idle 40000",
         "transform=grapevine_ripe",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 15, 18 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "reed_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed (medium)"),
   size = "small",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 40000",
         "transform=reed_ripe",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 13, 14 },
      },
   }
}
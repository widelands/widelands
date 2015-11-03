dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "grapevine_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Grapevine (tiny)"),
   size = "medium",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 22000",
         "transform=grapevine_small",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 15, 18 },
      },
   }
}

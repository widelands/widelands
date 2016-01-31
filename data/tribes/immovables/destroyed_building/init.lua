dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "destroyed_building",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Destroyed building"),
   size = "big",
   programs = {
      program = {
         "animate=idle 30000",
         "transform=ashes",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "burn_??.png"),
         hotspot = { 40, 57 },
         fps = 10,
      },
   }
}

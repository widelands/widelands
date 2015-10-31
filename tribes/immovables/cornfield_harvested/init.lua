dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cornfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (harvested)"),
   attributes = { "field" },
   programs = {
      disappear = {
         "animate=idle 50000",
         "remove=",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 31, 30 },
      },
   }
}

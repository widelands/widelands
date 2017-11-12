dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "field_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Field (tiny)"),
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "field", "seed_wheat" },

   -- NOCOM for testing only. Remove before merging.
   terrain_affinity = {
      preferred_temperature = 110,
      preferred_humidity = 0.4,
      preferred_fertility = 0.6,
      pickiness = 0.6,
   },

   programs = {
      program = {
         "animate=idle 30000",
         "transform=field_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 11, 5 },
      },
   }
}

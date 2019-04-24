dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (harvested)"),
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 1000000",
         "remove=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 34 },
      },
   }
}

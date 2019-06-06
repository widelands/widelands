dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (tiny)"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "field", "seed_barley" },
   programs = {
      program = {
         "animate=idle 150000",
         "transform=barleyfield_small",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 13 },
      },
   }
}

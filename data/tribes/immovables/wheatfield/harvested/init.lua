dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "wheatfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Wheat Field (harvested)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 500000",
         "transform=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 33, 22 },
      },
   }
}

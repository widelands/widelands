dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cornfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (harvested)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   attributes = { "field" },
   programs = {
      program = {
         "animate=idle 50000",
         "remove=",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 22 },
      },
   }
}

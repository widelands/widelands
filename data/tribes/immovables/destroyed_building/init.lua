dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "destroyed_building",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Destroyed building"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   programs = {
      program = {
         "animate=idle duration:30s",
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

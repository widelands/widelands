dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cassavarootfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cassavaroot Field (ripe)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   attributes = { "ripe_cassava", "field" },
   programs = {
      program = {
         "animate=idle 500000",
         "remove=",
      },
      harvest = {
         "transform=cassavarootfield_harvested",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 28, 45 },
      },
   }
}

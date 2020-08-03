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
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "transform=cassavarootfield_harvested",
      }
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {15, 36},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 2
      }
   }
}

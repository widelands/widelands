dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (harvested)"),
   icon = dirname .. "menu.png",
   helptext_script = dirname .. "helptexts.lua",
   programs = {
      main = {
         "animate=idle duration:16m40s",
         "remove=",
      }
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {21, 34}
      }
   }
}

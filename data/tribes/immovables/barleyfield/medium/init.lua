dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (medium)"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "field", "flowering" },
   programs = {
      program = {
         "animate=idle 250000",
         "transform=barleyfield_ripe",
      }
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {21, 33}
      }
   }
}

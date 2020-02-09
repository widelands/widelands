dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barleyfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Barley Field (ripe)"),
   size = "small",
   icon = dirname .. "menu.png",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_barley", "field" },
   programs = {
      program = {
         "animate=idle 2500000",
         "remove=",
      },
      harvest = {
         "transform=barleyfield_harvested"
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

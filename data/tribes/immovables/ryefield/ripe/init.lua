push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "ryefield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Rye Field (ripe)"),
   size = "small",
   icon = dirname .. "menu.png",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "ripe_rye", "field" },
   programs = {
      program = {
         "animate=idle 2500000",
         "remove=",
      },
      harvest = {
         "transform=ryefield_harvested"
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

pop_textdomain()

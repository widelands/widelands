dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_burning",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Burning Charcoal Stack"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = {},
   programs = {
      program = {
         "animate=idle 60000",
         "transform=pond_coal",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {19, 56},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   }
}

dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_dry",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Dry Pond"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "pond_dry" },
   programs = {
      main = {
         "animate=idle duration:6m40s",
         "remove=",
      },
      with_fish = {
         "transform=pond_growing",
      },
      with_stack = {
         "transform=pond_burning",
      }
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 5}
      }
   }
}

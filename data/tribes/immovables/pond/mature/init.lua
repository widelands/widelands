dirname = path.dirname (__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "pond_mature",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Pond With Fish"),
   icon = dirname .. "menu.png",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "pond_mature" },
   programs = {
      main = {
         "animate=idle duration:6m40s",
         "transform=pond_dry",
      },
      fall_dry = {
         "transform=pond_dry",
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 5}
      }
   }
}

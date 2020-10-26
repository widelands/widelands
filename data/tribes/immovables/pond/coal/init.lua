push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_immovable_type {
   name = "pond_coal",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Pond With Charcoal"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:8m20s",
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

pop_textdomain()

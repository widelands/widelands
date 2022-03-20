push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "pond_burning",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Burning Charcoal Stack"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:1m",
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

pop_textdomain()

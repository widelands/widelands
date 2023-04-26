push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "pond_dry",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Dry Pond"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
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
         hotspot = {8, 5}
      }
   }
}

pop_textdomain()

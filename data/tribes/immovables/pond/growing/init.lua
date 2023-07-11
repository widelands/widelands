push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "pond_growing",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Pond With Fish Spawn"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:2m20s",
         "transform=pond_mature",
      },
   },
   animations = {
      idle = {
         hotspot = {8, 5}
      }
   }
}

pop_textdomain()

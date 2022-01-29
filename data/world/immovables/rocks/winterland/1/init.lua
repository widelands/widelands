push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks1",
   descname = _("Rocks 1"),
   animation_directory = dirname,
   icon = dirname .. "menu1.png",
   size = "big",
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "rocks1",
         hotspot = { 38, 83 }
      },
   }
}

pop_textdomain()

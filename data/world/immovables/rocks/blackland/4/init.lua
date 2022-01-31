push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "blackland_rocks4",
   descname = _("Rocks 4"),
   animation_directory = dirname,
   icon = dirname .. "menu4.png",
   size = "big",
   programs = {
      shrink = {
         "transform=blackland_rocks3"
      }
   },
   animations = {
      idle = {
         basename = "rocks4",
         hotspot = { 30, 75 }
      },
   }
}

pop_textdomain()

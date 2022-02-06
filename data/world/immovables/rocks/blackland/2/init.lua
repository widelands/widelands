push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "blackland_rocks2",
   descname = _("Rocks 2"),
   animation_directory = dirname,
   icon = dirname .. "menu2.png",
   size = "big",
   programs = {
      shrink = {
         "transform=blackland_rocks1"
      }
   },
   animations = {
      idle = {
         basename = "rocks2",
         hotspot = { 30, 75 }
      },
   }
}

pop_textdomain()

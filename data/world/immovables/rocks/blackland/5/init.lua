push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "blackland_rocks5",
   descname = _("Rocks 5"),
   animation_directory = dirname,
   icon = dirname .. "menu5.png",
   size = "big",
   programs = {
      shrink = {
         "transform=blackland_rocks4"
      }
   },
   animations = {
      idle = {
         basename = "rocks5",
         hotspot = { 30, 75 }
      },
   }
}

pop_textdomain()

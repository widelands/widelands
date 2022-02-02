push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "desert_rocks5",
   descname = _("Rocks 5"),
   animation_directory = dirname,
   icon = dirname .. "menu5.png",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks4"
      }
   },
   animations = {
      idle = {
         basename = "rocks5",
         hotspot = { 38, 89 }
      },
   }
}

pop_textdomain()

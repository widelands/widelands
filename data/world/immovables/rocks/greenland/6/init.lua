push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "greenland_rocks6",
   descname = _("Rocks 6"),
   animation_directory = dirname,
   icon = dirname .. "menu6.png",
   size = "big",
   programs = {
      shrink = {
         "transform=greenland_rocks5"
      }
   },
   animations = {
      idle = {
         basename = "rocks6",
         hotspot = { 38, 91 }
      },
   }
}

pop_textdomain()

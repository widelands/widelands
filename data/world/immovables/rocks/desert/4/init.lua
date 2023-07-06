push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "desert_rocks4",
   descname = _("Rocks 4"),
   animation_directory = dirname,
   icon = dirname .. "menu4.png",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks3"
      }
   },
   animations = {
      idle = {
         basename = "rocks4",
         hotspot = { 40, 84 }
      },
   }
}

pop_textdomain()

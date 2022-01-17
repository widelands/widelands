push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "desert_rocks2",
   descname = _ "Rocks 2",
   animation_directory = dirname,
   icon = dirname .. "menu2.png",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks1"
      }
   },
   animations = {
      idle = {
         basename = "rocks2",
         hotspot = { 36, 86 }
      },
   }
}

pop_textdomain()

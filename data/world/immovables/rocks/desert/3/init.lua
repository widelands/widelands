push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "desert_rocks3",
   descname = _ "Rocks 3",
   animation_directory = dirname,
   icon = dirname .. "menu3.png",
   size = "big",
   programs = {
      shrink = {
         "transform=desert_rocks2"
      }
   },
   animations = {
      idle = {
         basename = "rocks3",
         hotspot = { 39, 90 }
      },
   }
}

pop_textdomain()

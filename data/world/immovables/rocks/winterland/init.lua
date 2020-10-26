push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks1",
   descname = _ "Rocks 1",
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

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks2",
   descname = _ "Rocks 2",
   animation_directory = dirname,
   icon = dirname .. "menu2.png",
   size = "big",
   programs = {
      shrink = {
         "transform=winterland_rocks1"
      }
   },
   animations = {
      idle = {
         basename = "rocks2",
         hotspot = { 36, 86 }
      },
   }
}

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks3",
   descname = _ "Rocks 3",
   animation_directory = dirname,
   icon = dirname .. "menu3.png",
   size = "big",
   programs = {
      shrink = {
         "transform=winterland_rocks2"
      }
   },
   animations = {
      idle = {
         basename = "rocks3",
         hotspot = { 42, 89 }
      },
   }
}

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks4",
   descname = _ "Rocks 4",
   animation_directory = dirname,
   icon = dirname .. "menu4.png",
   size = "big",
   programs = {
      shrink = {
         "transform=winterland_rocks3"
      }
   },
   animations = {
      idle = {
         basename = "rocks4",
         hotspot = { 44, 84 }
      },
   }
}

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks5",
   descname = _ "Rocks 5",
   animation_directory = dirname,
   icon = dirname .. "menu5.png",
   size = "big",
   programs = {
      shrink = {
         "transform=winterland_rocks4"
      }
   },
   animations = {
      idle = {
         basename = "rocks5",
         hotspot = { 42, 92 }
      },
   }
}

wl.Descriptions():new_immovable_type{
   name = "winterland_rocks6",
   descname = _ "Rocks 6",
   animation_directory = dirname,
   icon = dirname .. "menu6.png",
   size = "big",
   programs = {
      shrink = {
         "transform=winterland_rocks5"
      }
   },
   animations = {
      idle = {
         basename = "rocks6",
         hotspot = { 39, 90 }
      },
   }
}

pop_textdomain()

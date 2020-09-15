push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_immovable_type {
   name = "cocoa_tree_sapling",
   descname = _ "Cocoa Tree (sapling)",
   size = "small",
   icon = dirname .. "menu_sapling.png",
   programs = {
      main = {
         "animate=idle duration:80s",
         "remove=chance:7.81%",
         "transform=cocoa_tree_pole",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_sapling",
      hotspot = {11, 21},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

tribes:new_immovable_type {
   name = "cocoa_tree_pole",
   descname = _ "Cocoa Tree (pole)",
   size = "small",
   icon = dirname .. "menu_pole.png",
   programs = {
      main = {
         "animate=idle duration:60s",
         "remove=chance:5.86%",
         "transform=cocoa_tree_mature",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_pole",
      hotspot = {16, 44},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

tribes:new_immovable_type {
   name = "cocoa_tree_mature",
   descname = _ "Cocoa Tree (mature)",
   size = "small",
   icon = dirname .. "menu_mature.png",
   programs = {
      main = {
         "animate=idle duration:60s",
         "remove=chance:3.91%",
         "transform=cocoa_tree_old",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_mature",
      hotspot = {27, 68},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

tribes:new_immovable_type {
   name = "cocoa_tree_old",
   descname = _ "Cocoa Tree (old)",
   size = "small",
   icon = dirname .. "menu_old.png",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_old",
      hotspot = {27, 68},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

pop_textdomain()

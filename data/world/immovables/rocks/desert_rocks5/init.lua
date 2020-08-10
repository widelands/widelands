dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 38, 89 }
   },
}

world:new_immovable_type{
   name = "desert_rocks5",
   descname = _ "Rocks 5",
   icon = dirname .. "menu.png",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=desert_rocks4"
      }
   },
   animations = animations
}

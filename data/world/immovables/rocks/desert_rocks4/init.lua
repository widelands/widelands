dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 40, 84 }
   },
}

world:new_immovable_type{
   name = "desert_rocks4",
   descname = _ "Rocks 4",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=desert_rocks3"
      }
   },
   animations = animations
}

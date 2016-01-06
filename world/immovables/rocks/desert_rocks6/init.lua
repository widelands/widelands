dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 38, 91 }
   },
}

world:new_immovable_type{
   name = "desert_rocks6",
   descname = _ "Rocks 6",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=desert_rocks5"
      }
   },
   animations = animations
}

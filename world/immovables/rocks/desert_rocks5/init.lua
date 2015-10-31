dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 38, 89 }
   },
}

world:new_immovable_type{
   name = "desert_rocks5",
   descname = _ "Rocks 5",
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
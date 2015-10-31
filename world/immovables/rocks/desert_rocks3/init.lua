dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "desert_rocks3",
   descname = _ "Rocks 3",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=desert_rocks2"
      }
   },
   animations = animations
}

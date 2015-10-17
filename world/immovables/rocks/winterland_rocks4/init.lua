dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 44, 84 }
   },
}

world:new_immovable_type{
   name = "winterland_rocks4",
   descname = _ "Rocks 4",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=winterland_rocks3"
      }
   },
   animations = animations
}

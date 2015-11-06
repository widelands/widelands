dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 42, 92 }
   },
}

world:new_immovable_type{
   name = "winterland_rocks5",
   descname = _ "Rocks 5",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=winterland_rocks4"
      }
   },
   animations = animations
}

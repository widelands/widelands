dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "winterland_rocks6",
   descname = _ "Rocks 6",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=winterland_rocks5"
      }
   },
   animations = animations
}
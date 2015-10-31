dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks5",
   descname = _ "Rocks 5",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=greenland_rocks4"
      }
   },
   animations = animations
}
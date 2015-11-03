dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks3",
   descname = _ "Rocks 3",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=greenland_rocks2"
      }
   },
   animations = animations
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_rocks3",
   descname = _ "Rocks 3",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=blackland_rocks2"
      }
   },
   animations = animations
}

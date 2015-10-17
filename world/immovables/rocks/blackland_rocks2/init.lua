dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle",
      directory = dirname,
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_rocks2",
   descname = _ "Rocks 2",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=blackland_rocks1"
      }
   },
   animations = animations
}

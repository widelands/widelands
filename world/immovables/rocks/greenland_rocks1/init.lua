dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 39, 82 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks1",
   descname = _ "Rocks 1",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = animations
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "winterland_stones6",
   descname = _ "Stones 6",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=winterland_stones5"
      }
   },
   animations = animations
}

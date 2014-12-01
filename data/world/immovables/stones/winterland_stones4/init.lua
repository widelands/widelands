dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 44, 84 }
   },
}

world:new_immovable_type{
   name = "winterland_stones4",
   descname = _ "Stones 4",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=winterland_stones3"
      }
   },
   animations = animations
}

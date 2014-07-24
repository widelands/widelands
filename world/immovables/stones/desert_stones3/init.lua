dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "desert_stones3",
   descname = _ "Stones 3",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=desert_stones2"
      }
   },
   animations = animations
}

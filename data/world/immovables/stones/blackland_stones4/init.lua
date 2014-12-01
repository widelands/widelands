dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_stones4",
   descname = _ "Stones 4",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=blackland_stones3"
      }
   },
   animations = animations
}

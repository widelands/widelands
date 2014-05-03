dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_stones3",
   descname = _ "Stones 3",
   editor_category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=blackland_stones2"
      }
   },
   animations = animations
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_stones3",
   descname = _ "Stones 3",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=greenland_stones2"
      }
   },
   animations = animations
}

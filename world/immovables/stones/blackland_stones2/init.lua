dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_stones2",
   descname = _ "Stones 2",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "transform=blackland_stones1"
      }
   },
   animations = animations
}

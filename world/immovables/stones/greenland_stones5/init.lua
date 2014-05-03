dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_stones5",
   descname = _ "Stones 5",
   editor_category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=greenland_stones4"
      }
   },
   animations = animations
}

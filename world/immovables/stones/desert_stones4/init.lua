dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 40, 84 }
   },
}

world:new_immovable_type{
   name = "desert_stones4",
   descname = _ "Stones 4",
   -- category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=desert_stones3"
      }
   },
   animations = animations
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 42, 92 }
   },
}

world:new_immovable_type{
   name = "winterland_stones5",
   descname = _ "Stones 5",
   -- category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=winterland_stones4"
      }
   },
   animations = animations
}

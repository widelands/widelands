dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 39, 82 }
   },
}

world:new_immovable_type{
   name = "greenland_stones1",
   descname = _ "Stones 1",
   -- category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = animations
}

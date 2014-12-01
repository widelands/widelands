dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      hotspot = { 38, 83 }
   },
}

world:new_immovable_type{
   name = "winterland_stones1",
   descname = _ "Stones 1",
   editor_category = "stones",
   size = "big",
   attributes = { "granite" },
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = animations
}

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 38, 91 }
   },
}

world:new_immovable_type{
   name = "greenland_stones6",
   descname = _ "Stones 6",
   -- category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=greenland_stones5"
      }
   },
   animations = animations
}

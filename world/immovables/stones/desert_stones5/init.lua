dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 38, 89 }
   },
}

world:new_immovable_type{
   name = "desert_stones5",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=desert_stones4"
      }
   },
   animations = animations
}

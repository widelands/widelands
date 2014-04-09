dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 40, 84 }
   },
}

world:new_immovable_type{
   name = "desert_stones4",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=desert_stones3"
      }
   },
   animations = animations
}

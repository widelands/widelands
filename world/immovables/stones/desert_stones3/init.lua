dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "desert_stones3",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=desert_stones2"
      }
   },
   animations = animations
}


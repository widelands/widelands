dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 42, 89 }
   },
}

world:new_immovable_type{
   name = "winterland_stones3",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=winterland_stones2"
      }
   },
   animations = animations
}

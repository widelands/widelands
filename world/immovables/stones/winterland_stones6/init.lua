dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 39, 90 }
   },
}

world:new_immovable_type{
   name = "winterland_stones6",
   descname = _ "Stones 6",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=winterland_stones5"
      }
   },
   animations = animations
}

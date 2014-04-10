dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_stones5",
   descname = _ "Stones 5",
   category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=blackland_stones4"
      }
   },
   animations = animations
}

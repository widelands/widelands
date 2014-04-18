dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_stones3",
   descname = _ "Stones 3",
   -- category = "stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=greenland_stones2"
      }
   },
   animations = animations
}

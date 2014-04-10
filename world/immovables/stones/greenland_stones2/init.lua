dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 36, 86 }
   },
}

world:new_immovable_type{
   name = "greenland_stones2",
   descname = _ "Stones 2",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=greenland_stones1"
      }
   },
   animations = animations
}

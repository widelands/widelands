dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_stones5",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "transform=greenland_stones4"
      }
   },
   animations = animations
}

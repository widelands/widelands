dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 38, 82 }
   },
}

world:new_immovable_type{
   name = "desert_stones1",
   descname = _ "Stones 1",
   size = "big",
   attributes = { "stone" },
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = animations
}

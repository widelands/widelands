dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone4_summer",
   descname = _ "Standing Stone",
   category = "standing_stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 11, 74 },
      },
   }
}

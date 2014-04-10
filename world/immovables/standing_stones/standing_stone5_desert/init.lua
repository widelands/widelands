dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone5_desert",
   descname = _ "Standing Stone",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 37, 27 },
      },
   }
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone2_desert",
   descname = _ "Standing Stone",
   -- category = "standing_stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         player_color_masks = {},
         hotspot = { 20, 38 },
      },
   }
}

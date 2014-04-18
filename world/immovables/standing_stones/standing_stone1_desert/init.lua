dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone1_desert",
   descname = _ "Standing Stone",
   -- category = "standing_stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 18, 49 },
      },
   }
}

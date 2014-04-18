dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone7",
   descname = _ "Standing Stone",
   -- category = "standing_stones",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 63 },
      },
   }
}

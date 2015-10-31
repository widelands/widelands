dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone2_desert",
   descname = _ "Standing Stone",
   editor_category = "standing_stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 20, 38 },
      },
   }
}
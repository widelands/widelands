dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone2_summer",
   descname = _ "Standing Stone",
   editor_category = "standing_stones",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 20, 38 },
      },
   }
}

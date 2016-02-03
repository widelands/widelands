dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone6",
   descname = _ "Standing Stone",
   editor_category = "standing_stones",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 63 },
      },
   }
}

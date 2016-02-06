dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone4_winter",
   descname = _ "Standing Stone",
   editor_category = "standing_stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 12, 75 },
      },
   }
}

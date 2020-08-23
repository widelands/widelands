dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "standing_stone3_winter",
   descname = _ "Standing Stone",
   editor_category = "standing_stones",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 25, 28 },
      },
   }
}

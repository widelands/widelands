dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton3",
   descname = _ "Skeleton",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 47, 69 },
      },
   }
}

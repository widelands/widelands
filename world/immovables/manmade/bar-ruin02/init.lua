dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin02",
   descname = _ "Ruin",
   editor_category = "miscellaneous",
   size = "medium",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 43, 43 },
      },
   }
}

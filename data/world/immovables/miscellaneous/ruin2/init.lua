dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "ruin2",
   descname = _ "Ruin",
   editor_category = "miscellaneous",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 29, 36 },
      },
   }
}

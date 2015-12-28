dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom2",
   descname = _ "Mushroom",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 7 },
      },
   }
}

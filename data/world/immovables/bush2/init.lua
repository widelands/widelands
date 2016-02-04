dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush2",
   descname = _ "Bush",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 12, 9 },
      },
   }
}

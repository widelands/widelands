dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush5",
   descname = _ "Bush",
   editor_category = "plants",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 14, 13 },
      },
   }
}

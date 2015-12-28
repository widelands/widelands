dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "grass1",
   descname = _ "Grass",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 20 },
      },
   }
}

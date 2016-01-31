dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cactus2",
   descname = _ "Cactus",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 17, 25 },
      },
   }
}

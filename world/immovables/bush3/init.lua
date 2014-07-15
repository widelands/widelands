dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush3",
   descname = _ "Bush",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 11, 14 },
      },
   }
}

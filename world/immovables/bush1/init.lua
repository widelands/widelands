dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush1",
   descname = _ "Bush",
   -- category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 9 },
      },
   }
}

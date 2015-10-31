dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush4",
   descname = _ "Bush",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 12, 10 },
      },
   }
}

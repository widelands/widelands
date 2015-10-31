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
         template = "idle",
         directory = dirname,
         hotspot = { 10, 20 },
      },
   }
}
dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "debris02",
   descname = _ "Debris",
   editor_category = "miscellaneous",
   size = "small",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 35, 35 },
      },
   }
}

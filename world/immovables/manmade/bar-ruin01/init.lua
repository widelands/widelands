dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin01",
   descname = _ "Ruin",
   editor_category = "miscellaneous",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 64, 53 },
      },
   }
}
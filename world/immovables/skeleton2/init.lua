dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton2",
   descname = _ "Seashell",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 26, 32 },
      },
   }
}
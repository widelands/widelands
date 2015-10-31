dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble4",
   descname = _ "Pebble",
   editor_category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 5, 5 }
      },
   }
}

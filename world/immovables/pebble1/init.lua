dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble1",
   descname = _ "Pebble",
   -- category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 4 },
      },
   }
}

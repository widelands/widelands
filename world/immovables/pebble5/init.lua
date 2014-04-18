dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble5",
   descname = _ "Pebble",
   -- category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 6 },
      },
   }
}

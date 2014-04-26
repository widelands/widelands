dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin00",
   descname = _ "Ruin",
   -- category = "miscellaneous",
   size = "small",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 34, 32 },
      },
   }
}

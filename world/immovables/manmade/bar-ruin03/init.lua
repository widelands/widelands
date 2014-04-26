dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin03",
   descname = _ "Ruin",
   -- category = "miscellaneous",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 44, 34 },
      },
   }
}

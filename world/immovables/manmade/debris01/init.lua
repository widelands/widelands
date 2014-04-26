dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "debris01",
   descname = _ "Debris",
   -- category = "miscellaneous",
   size = "small",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 35, 35 },
      },
   }
}

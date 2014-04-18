dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cactus4",
   descname = _ "Cactus",
   -- category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 71 },
      },
   }
}

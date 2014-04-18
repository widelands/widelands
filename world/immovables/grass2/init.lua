dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "grass2",
   descname = _ "Grass",
   -- category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 16 },
      },
   }
}

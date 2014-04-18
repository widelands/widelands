dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cactus3",
   descname = _ "Cactus",
   -- category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         player_color_masks = {},
         hotspot = { 10, 71 },
      },
   }
}

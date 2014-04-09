dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton3",
   descname = _ "Skeleton",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 47, 69 },
      },
   }
}

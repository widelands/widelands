dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton1",
   descname = _ "Skeleton 1",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 29, 36 },
      },
   }
}

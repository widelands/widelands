dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "sstones15",
   descname = _ "Standing Stones",
   size = "big",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 25, 28 },
      },
   }
}

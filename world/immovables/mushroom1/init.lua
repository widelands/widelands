dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom1",
   descname = _ "Mushroom",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 6, 9 },
      },
   }
}

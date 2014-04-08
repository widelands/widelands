dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble4",
   descname = _ "Pebble",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 5, 5 }
      },
   }
}

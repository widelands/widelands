dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bush2",
   descname = _ "Bush",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 12, 9 },
      },
   }
}


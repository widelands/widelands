dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "snowman",
   descname = _ "Snowman",
   category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 9, 24 },
      },
   }
}

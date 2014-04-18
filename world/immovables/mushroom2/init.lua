dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom2",
   descname = _ "Mushroom",
   category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         player_color_masks = {},
         hotspot = { 5, 7 },
      },
   }
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "skeleton4",
   descname = _ "Skeleton",
   -- category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         player_color_masks = {},
         hotspot = { 26, 32 },
      },
   }
}

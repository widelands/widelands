dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "snowman",
   descname = _ "Snowman",
   -- NOCOM: I don't know if these are correct values for size, attributes and programs, but the editor refused to start without these fields.
   size = "small",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 9, 24 },
         fps = 1,
      },
   }
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "birch_summer_pole",
   descname = _ "Birch (Pole)",
   category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=birch_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 12, 28 },
         fps = 8,
         sfx = {},
      },
   },
}

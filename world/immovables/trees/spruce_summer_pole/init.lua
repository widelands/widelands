dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "spruce_summer_pole",
   descname = _ "Spruce (Pole)",
   category = "trees_coniferous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=spruce_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 9, 28 },
         fps = 8,
         sfx = {},
      },
   },
}

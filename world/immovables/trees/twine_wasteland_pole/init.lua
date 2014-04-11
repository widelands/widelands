dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "twine_wasteland_pole",
   descname = _ "Twine Tree (Pole)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=spruce_wasteland_mature",
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

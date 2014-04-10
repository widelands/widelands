dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_wasteland_pole",
   descname = _ "Oak (Pole)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 72500",
         "remove=70",
         "grow=oak_wasteland_mature",
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

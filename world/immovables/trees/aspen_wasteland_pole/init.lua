dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_wasteland_pole",
   descname = _ "Aspen (Pole)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=aspen_wasteland_mature",
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

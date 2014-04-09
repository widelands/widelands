dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_pole",
   descname = _ "Coconut Palm (Pole)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=palm_coconut_desert_mature",
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

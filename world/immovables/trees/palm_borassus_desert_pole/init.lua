dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_borassus_desert_pole",
   descname = _ "Borassus Palm (Pole)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 70000",
         "remove=70",
         "grow=palm_borassus_desert_mature",
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

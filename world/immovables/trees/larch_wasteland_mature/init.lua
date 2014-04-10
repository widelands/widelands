dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_wasteland_mature",
   descname = _ "Larch (Mature)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=24",
         "grow=larch_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 48 },
         fps = 8,
         sfx = {},
      },
   },
}

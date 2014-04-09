dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_wasteland_mature",
   descname = _ "Aspen (Mature)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=aspen_wasteland_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=aspen_wasteland_old",
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

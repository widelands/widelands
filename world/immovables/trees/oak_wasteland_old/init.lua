dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_wasteland_old",
   descname = _ "Oak (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 25",
         "seed=oak_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 24, 60 },
         fps = 15,
         sfx = {},
      },
   },
}

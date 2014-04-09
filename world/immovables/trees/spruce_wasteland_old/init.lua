dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "spruce_wasteland_old",
   descname = _ "Spruce (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree3 36",
         "seed=spruce_wasteland_sapling",
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
         fps = 10,
         sfx = {},
      },
   },
}

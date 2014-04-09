dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_wasteland_sapling",
   descname = _ "Oak (Sapling)",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 72500",
         "remove=80",
         "grow=oak_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 5, 12 },
         fps = 8,
         sfx = {},
      },
   },
}

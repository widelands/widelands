dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_wasteland_old",
   descname = _ "Aspen (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 32",
         "seed=aspen_wasteland_sapling",
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
         sfx = {
            [0] = "sound/animals/bird1.ogg",
         },
      },
   },
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "birch_wasteland_old",
   descname = _ "Birch (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree2 50",
         "seed=birch_wasteland_sapling",
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
            [0] = "sound/animals/bird4.ogg",
         },
      },
   },
}

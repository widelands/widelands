dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "beech_summer_old",
   descname = _ "Beech (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1525000",
         "transform=deadtree2 20",
         "seed=beech_summer_sapling",
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
            [0] = "sound/animals/bird6.ogg",
         },
      },
   },
}

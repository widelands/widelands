dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_roystonea_desert_old",
   descname = _ "Roystonea regia Palm (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 39",
         "seed=palm_roystonea_desert_sapling",
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

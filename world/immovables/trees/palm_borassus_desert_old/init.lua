dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_borassus_desert_old",
   descname = _ "Borassus Palm (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 2000000",
         "transform=deadtree5 25",
         "seed=palm_borassus_desert_sapling",
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
            [0] = "sound/animals/bird2.ogg",
         },
      },
   },
}

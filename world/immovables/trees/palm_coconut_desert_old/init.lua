dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_old",
   descname = _ "Coconut Palm (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree6 36",
         "seed=palm_coconut_desert_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 24, 59 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird3.ogg",
         },
      },
   },
}
--mountain4=            2

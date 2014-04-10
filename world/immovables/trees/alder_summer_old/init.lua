dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_old",
   descname = _ "Alder (Old)",
   category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 5",
         "seed=alder_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 23, 59 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird4.ogg",
         },
      },
   },
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "birch_summer_old",
   descname = _ "Birch (Old)",
   category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree2 27",
         "seed=birch_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 23, 58 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird5.ogg",
         },
      },
   },
}

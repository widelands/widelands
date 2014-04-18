dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "maple_winter_old",
   descname = _ "Maple (Old)",
   category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 39",
         "seed=maple_winter_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 23, 59 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird4.ogg",
         },
      },
   },
}

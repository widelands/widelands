dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "rowan_summer_old",
   descname = _ "Rowan (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1432000",
         "transform=deadtree4 26",
         "seed=rowan_summer_sapling",
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
            [0] = "sound/animals/bird6.ogg",
         },
      },
   },
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_summer_old",
   descname = _ "Larch (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 23",
         "seed=larch_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 15, 59 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird6.ogg",
         },
      },
   },
}

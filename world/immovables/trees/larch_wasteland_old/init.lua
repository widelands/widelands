dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_wasteland_old",
   descname = _ "Larch (Old)",
   category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 34",
         "seed=larch_wasteland_sapling",
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
         sfx = {},
      },
   },
}

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cirrus_wasteland_sapling",
   descname = _ " Cirrus Tree (Sapling)",
   category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 61000",
         "remove=44",
         "grow=larch_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 5, 12 },
         fps = 8,
         sfx = {},
      },
   },
}

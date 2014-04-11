dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_red_wasteland_mature",
   descname = _ "Red Mushroom Tree (Mature)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 27000",
         "remove=10",
         "seed=birch_wasteland_sapling",
         "animate=idle 29000",
         "remove=10",
         "grow=birch_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 48 },
         fps = 8,
         sfx = {},
      },
   },
}

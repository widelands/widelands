dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "rowan_wasteland_mature",
   descname = _ "Rowan (Mature)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=10",
         "seed=rowan_wasteland_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=rowan_wasteland_old",
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

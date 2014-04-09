dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_oil_desert_mature",
   descname = _ "Oil Palm (Mature)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 25000",
         "remove=10",
         "seed=palm_oil_desert_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=palm_oil_desert_old",

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

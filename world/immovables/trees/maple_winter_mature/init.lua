dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "maple_winter_mature",
   descname = _ "Maple (Mature)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "seed=maple_winter_sapling",
         "grow=maple_winter_old",
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

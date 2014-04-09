dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 60000",
         "remove=44",
         "grow=larch_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 4, 12 },
         fps = 8,
         sfx = {},
      },
   },
}

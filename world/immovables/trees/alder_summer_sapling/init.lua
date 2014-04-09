dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "grow=alder_summer_pole",
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

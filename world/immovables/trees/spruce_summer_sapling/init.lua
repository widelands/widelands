dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "spruce_summer_sapling",
   descname = _ "Spruce (Sapling)",
   size = "small",
   attributes = { "tree", "seed" },
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=spruce_summer_pole",
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

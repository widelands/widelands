dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_sapling",
   descname = _ "Coconut Palm (Sapling)",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=palm_coconut_desert_pole",
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

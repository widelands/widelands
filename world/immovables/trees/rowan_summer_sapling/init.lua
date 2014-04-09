dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "rowan_summer_sapling",
   descname = _ "Rowan (Sapling)",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 60000",
         "remove=40",
         "grow=rowan_summer_pole",
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

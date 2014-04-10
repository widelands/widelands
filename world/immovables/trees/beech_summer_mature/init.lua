dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "beech_summer_mature",
   descname = _ "Beech (Mature)",
   category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=beech_summer_old",
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

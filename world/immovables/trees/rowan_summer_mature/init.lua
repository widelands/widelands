dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "rowan_summer_mature",
   descname = _ "Rowan (Mature)",
   category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=7",
         "seed=rowan_summer_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=rowan_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 48 },
         fps = 8,
         sfx = {},
      },
   },
}

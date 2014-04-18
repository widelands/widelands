dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "maple_winter_pole",
   descname = _ "Maple (Pole)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "grow=maple_winter_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

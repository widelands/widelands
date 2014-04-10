dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "maple_wasteland_mature",
   descname = _ "Maple (Mature)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=maple_wasteland_old",
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

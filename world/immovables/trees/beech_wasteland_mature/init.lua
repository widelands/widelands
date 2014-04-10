dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "beech_wasteland_mature",
   descname = _ "Beech (Mature)",
   category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=beech_wasteland_old",
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

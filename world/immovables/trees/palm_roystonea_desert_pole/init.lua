dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_roystonea_desert_pole",
   descname = _ "Roystonea regia Palm (Pole)",
   category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "grow=palm_roystonea_desert_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 12, 28 },
         fps = 8,
         sfx = {},
      },
   },
}

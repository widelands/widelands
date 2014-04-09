dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_roystonea_desert_mature",
   descname = _ "Roystonea regia Palm (Mature)",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=palm_roystonea_desert_old",
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
--mountain4=            2

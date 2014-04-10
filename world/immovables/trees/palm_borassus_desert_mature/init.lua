dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_borassus_desert_mature",
   descname = _ "Borassus Palm (Mature)",
   category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 65000",
         "remove=40",
         "grow=palm_borassus_desert_old",
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

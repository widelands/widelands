dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_date_desert_pole",
   descname = _ "Date Palm (Pole)",
   category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=palm_date_desert_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 12, 28 },
         fps = 8,
         sfx = {},
      },
   },
}

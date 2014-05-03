dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "umbrella_red_wasteland_pole",
   descname = _ "Red Umbrella Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 38000",
         "remove=47",
         "grow=aspen_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

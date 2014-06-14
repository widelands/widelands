dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cirrus_wasteland_mature",
   descname = _ "Cirrus Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=24",
         "grow=cirrus_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

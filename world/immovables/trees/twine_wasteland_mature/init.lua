dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "twine_wasteland_mature",
   descname = _ "Twine Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 60000",
         "remove=23",
         "grow=spruce_wasteland_old",
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

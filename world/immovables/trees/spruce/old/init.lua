dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "spruce_summer_old",
   descname = _ "Spruce (Old)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree3 24",
         "seed=spruce_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 15, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird3",
         },
      },
   },
}

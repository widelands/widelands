dirname = path.dirname(__file__)

-- NOCOM(#sirver): these are concept values and all the same for all trees right now.
terrain_affinity = {
   -- In Kelvin.
   preferred_temperature = 289.65,

   -- In percent (1 being very wet).
   preferred_humidity = 0.66,

   -- In percent (1 being very fertile).
   preferred_fertility = 0.9,

   -- NOCOM(#sirver): figure this out. I imagine a scaling factor for the sigma of the gaussian.
   pickiness = 1.,
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_sapling",
   descname = _ "Dark Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=80",
         "grow=mushroom_dark_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_pole",
   descname = _ "Dark Mushroom Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=70",
         "grow=mushroom_dark_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_mature",
   descname = _ "Dark Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 75000",
         "remove=40",
         "grow=mushroom_dark_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_old",
   descname = _ "Dark Mushroom Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 25",
         "seed=mushroom_dark_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 15,
      },
   },
}

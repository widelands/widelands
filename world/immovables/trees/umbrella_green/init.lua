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
   name = "umbrella_green_wasteland_sapling",
   descname = _ "Green Umbrella Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 57000",
         "remove=21",
         "grow=umbrella_green_wasteland_pole",
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
   name = "umbrella_green_wasteland_pole",
   descname = _ "Green Umbrella Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 53000",
         "remove=19",
         "grow=umbrella_green_wasteland_mature",
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
   name = "umbrella_green_wasteland_mature",
   descname = _ "Green Umbrella Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=umbrella_green_wasteland_old",
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
   name = "umbrella_green_wasteland_old",
   descname = _ "Green Umbrella Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 39",
         "seed=umbrella_green_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}

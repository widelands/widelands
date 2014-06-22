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
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=44",
         "grow=larch_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 4, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_pole",
   descname = _ "Larch (Pole)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=34",
         "grow=larch_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 9, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=24",
         "grow=larch_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 12, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_old",
   descname = _ "Larch (Old)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 23",
         "seed=larch_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 15, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird6",
         },
      },
   },
}

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 600,
   preferred_fertility = 650,
   pickiness = 35,
}

terrain_affinity_black = {
   preferred_temperature = 100,
   preferred_humidity = 150,
   preferred_fertility = 850,
   pickiness = 50,
}

terrain_affinity_desert = {
   preferred_temperature = 140,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 50,
}

terrain_affinity_winter = {
   preferred_temperature = 45,
   preferred_humidity = 750,
   preferred_fertility = 400,
   pickiness = 50,
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_amazons_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_amazons_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_amazons_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   icon = dirname .. "old/idle_0.png",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_amazons_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_black_amazons_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_black_amazons_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_black_amazons_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   icon = dirname .. "old/idle_0.png",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_black_amazons_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_desert_amazons_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_desert_amazons_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_desert_amazons_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   icon = dirname .. "old/idle_0.png",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_desert_amazons_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_winter_amazons_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_winter_amazons_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_winter_amazons_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   icon = dirname .. "old/idle_0.png",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_winter_amazons_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}

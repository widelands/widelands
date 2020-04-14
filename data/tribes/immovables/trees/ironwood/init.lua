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

spritesheet_sapling = { idle = {
   basename = "ironwood_sapling",
   hotspot = {8, 21},
   fps = 8,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_pole = { idle = {
   basename = "ironwood_pole",
   hotspot = {13, 46},
   fps = 8,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_mature = { idle = {
   basename = "ironwood_mature",
   hotspot = {18, 69},
   fps = 8,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_old = {
   idle = {
      basename = "ironwood_old",
      hotspot = {18, 77},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   },
   fall = {
      basename = "ironwood_fall",
      hotspot = {19, 83},
      fps = 10,
      frames = 10,
      columns = 5,
      rows = 2,
      play_once = true
   }
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=ironwood_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=ironwood_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=ironwood_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Ironwood Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood", "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=ironwood_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_black_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood_sapling" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=ironwood_black_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_black_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=ironwood_black_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_black_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=ironwood_black_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_black_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Ironwood Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood", "tree" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=ironwood_black_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_desert_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood_sapling" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=ironwood_desert_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_desert_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=ironwood_desert_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_desert_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=ironwood_desert_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_desert_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Ironwood Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood", "tree" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=ironwood_desert_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_winter_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood_sapling" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=ironwood_winter_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_winter_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=ironwood_winter_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_winter_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=ironwood_winter_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "ironwood_winter_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Ironwood Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_ironwood", "tree" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=ironwood_winter_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

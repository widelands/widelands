push_textdomain("tribes")

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

descriptions:new_immovable_type{
   name = "ironwood_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=ironwood_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

descriptions:new_immovable_type{
   name = "ironwood_amazons_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=ironwood_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

descriptions:new_immovable_type{
   name = "ironwood_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

descriptions:new_immovable_type{
   name = "ironwood_amazons_old",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is an amazon rare tree.
   species = _ "Ironwood Tree",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=ironwood_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

descriptions:new_immovable_type{
   name = "ironwood_black_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=ironwood_black_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

descriptions:new_immovable_type{
   name = "ironwood_black_amazons_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=ironwood_black_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

descriptions:new_immovable_type{
   name = "ironwood_black_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_black_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

descriptions:new_immovable_type{
   name = "ironwood_black_amazons_old",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is an amazon rare tree.
   species = _ "Ironwood Tree",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=ironwood_black_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

descriptions:new_immovable_type{
   name = "ironwood_desert_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=ironwood_desert_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

descriptions:new_immovable_type{
   name = "ironwood_desert_amazons_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=ironwood_desert_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

descriptions:new_immovable_type{
   name = "ironwood_desert_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_desert_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

descriptions:new_immovable_type{
   name = "ironwood_desert_amazons_old",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is an amazon rare tree.
   species = _ "Ironwood Tree",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=ironwood_desert_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

descriptions:new_immovable_type{
   name = "ironwood_winter_amazons_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=ironwood_winter_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

descriptions:new_immovable_type{
   name = "ironwood_winter_amazons_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=ironwood_winter_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

descriptions:new_immovable_type{
   name = "ironwood_winter_amazons_mature",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=ironwood_winter_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

descriptions:new_immovable_type{
   name = "ironwood_winter_amazons_old",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _ "Ironwood Tree (Old)",
   -- TRANSLATORS:  This is an amazon rare tree.
   species = _ "Ironwood Tree",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=ironwood_winter_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

pop_textdomain()

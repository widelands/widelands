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
   basename = "balsa_sapling",
   hotspot = {7, 16},
   fps = 3,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_pole = { idle = {
   basename = "balsa_pole",
   hotspot = {12, 31},
   fps = 3,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_mature = { idle = {
   basename = "balsa_mature",
   hotspot = {18, 60},
   fps = 3,
   frames = 4,
   columns = 2,
   rows = 2
}}

spritesheet_old = {
   idle = {
      basename = "balsa_old",
      hotspot = {20, 67},
      fps = 3,
      frames = 4,
      columns = 2,
      rows = 2
   },
   fall = {
      basename = "balsa_fall",
      hotspot = {20, 73},
      fps = 10,
      frames = 10,
      columns = 5,
      rows = 2,
      play_once = true
   }
}

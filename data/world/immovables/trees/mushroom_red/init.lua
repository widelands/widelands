push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 80,
   preferred_humidity = 350,
   preferred_fertility = 850,
   pickiness = 60,
}

wl.World():new_immovable_type{
   name = "mushroom_red_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Mushroom Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:42s",
         "remove=chance:12.5%",
         "grow=mushroom_red_wasteland_pole",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "sapling",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 6, 13 }
      }
   },
}

wl.World():new_immovable_type{
   name = "mushroom_red_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Mushroom Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:40s",
         "remove=chance:9.76%",
         "grow=mushroom_red_wasteland_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 13, 29 }
      }
   },
}

wl.World():new_immovable_type{
   name = "mushroom_red_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Mushroom Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:27s",
         "remove=chance:3.91%",
         "seed=mushroom_red_wasteland_sapling proximity:39%",
         "animate=idle duration:29s",
         "remove=chance:3.91%",
         "grow=mushroom_red_wasteland_old",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 19, 49 }
      }
   },
}

wl.World():new_immovable_type{
   name = "mushroom_red_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Mushroom Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   species = _ "Red Mushroom Tree",
   icon = dirname .. "menu.png",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:13m20s",
         "transform=deadtree2 chance:19.53%",
         "seed=mushroom_red_wasteland_sapling proximity:15.62%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 25, 61 }
      }
   },
}

pop_textdomain()

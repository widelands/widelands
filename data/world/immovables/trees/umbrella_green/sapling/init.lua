push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 200,
   preferred_fertility = 850,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "umbrella_green_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:57s",
         "remove=chance:8.20%",
         "grow=umbrella_green_wasteland_pole",
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
         hotspot = { 5, 12 }
      }
   },
}

pop_textdomain()

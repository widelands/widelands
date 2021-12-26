push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 200,
   preferred_fertility = 850,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "umbrella_green_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Green Umbrella Tree (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:53s",
         "remove=chance:7.42%",
         "grow=umbrella_green_wasteland_mature",
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
         hotspot = { 12, 28 }
      }
   },
}

pop_textdomain()

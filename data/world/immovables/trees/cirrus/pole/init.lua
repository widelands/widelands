push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 150,
   preferred_fertility = 950,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "cirrus_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Cirrus Tree (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:59s",
         "remove=chance:13.28%",
         "grow=cirrus_wasteland_mature",
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

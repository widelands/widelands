push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 150,
   preferred_humidity = 550,
   preferred_fertility = 500,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "palm_oil_desert_pole",
   descname = _ "Oil Palm (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:40s",
         "remove=chance:9.76%",
         "grow=palm_oil_desert_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 12, 28 }
      }
   },
}

pop_textdomain()

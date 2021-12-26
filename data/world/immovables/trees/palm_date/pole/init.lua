push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 160,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "palm_date_desert_pole",
   descname = _ "Date Palm (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:18.36%",
         "grow=palm_date_desert_mature",
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

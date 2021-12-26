push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 160,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "palm_date_desert_sapling",
   descname = _ "Date Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:19.53%",
         "grow=palm_date_desert_pole",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "sapling",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 5, 12 }
      }
   },
}

pop_textdomain()

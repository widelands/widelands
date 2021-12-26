push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 180,
   preferred_humidity = 400,
   preferred_fertility = 400,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_borassus_desert_sapling",
   descname = _ "Borassus Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "remove=chance:31.25%",
         "grow=palm_borassus_desert_pole",
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

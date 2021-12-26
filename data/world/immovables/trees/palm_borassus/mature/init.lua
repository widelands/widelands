push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 180,
   preferred_humidity = 400,
   preferred_fertility = 400,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_borassus_desert_mature",
   descname = _ "Borassus Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m5s",
         "remove=chance:15.62%",
         "grow=palm_borassus_desert_old",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "mature",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 18, 48 }
      }
   },
}

pop_textdomain()

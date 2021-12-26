push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 160,
   preferred_humidity = 600,
   preferred_fertility = 600,
   pickiness = 90,
}

wl.Descriptions():new_immovable_type{
   name = "palm_roystonea_desert_mature",
   descname = _ "Roystonea regia Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=chance:7.03%",
         "grow=palm_roystonea_desert_old",
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

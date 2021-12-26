push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 180,
   preferred_humidity = 400,
   preferred_fertility = 400,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_borassus_desert_pole",
   descname = _ "Borassus Palm (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m10s",
         "remove=chance:27.34%",
         "grow=palm_borassus_desert_mature",
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

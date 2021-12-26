push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 130,
   preferred_humidity = 500,
   preferred_fertility = 600,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_coconut_desert_mature",
   descname = _ "Coconut Palm (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m",
         "remove=chance:8.98%",
         "grow=palm_coconut_desert_old",
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

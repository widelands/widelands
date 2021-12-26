push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 400,
   preferred_fertility = 600,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "beech_summer_pole",
   descname = _ "Beech (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m5s",
         "remove=chance:9.37%",
         "grow=beech_summer_mature",
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

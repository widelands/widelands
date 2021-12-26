push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 550,
   preferred_fertility = 800,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "maple_winter_mature",
   descname = _ "Maple (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:50s",
         "remove=chance:7.03%",
         "grow=maple_winter_old",
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

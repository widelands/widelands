push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 130,
   preferred_humidity = 500,
   preferred_fertility = 600,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_coconut_desert_pole",
   descname = _ "Coconut Palm (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:12.89%",
         "grow=palm_coconut_desert_mature",
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

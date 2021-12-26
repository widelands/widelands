push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 130,
   preferred_humidity = 500,
   preferred_fertility = 600,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "palm_coconut_desert_sapling",
   descname = _ "Coconut Palm (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:16.41%",
         "grow=palm_coconut_desert_pole",
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

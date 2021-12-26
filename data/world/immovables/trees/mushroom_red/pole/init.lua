push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 80,
   preferred_humidity = 350,
   preferred_fertility = 850,
   pickiness = 60,
}

wl.Descriptions():new_immovable_type{
   name = "mushroom_red_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Red Mushroom Tree (Pole)",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:40s",
         "remove=chance:9.76%",
         "grow=mushroom_red_wasteland_mature",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "pole",
         fps = 8,
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 12, 28 }
      }
   },
}

pop_textdomain()

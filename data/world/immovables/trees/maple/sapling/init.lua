push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 110,
   preferred_humidity = 550,
   preferred_fertility = 800,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "maple_winter_sapling",
   descname = _ "Maple (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:57s500ms",
         "remove=chance:8.20%",
         "grow=maple_winter_pole",
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

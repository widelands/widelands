push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 160,
   preferred_humidity = 600,
   preferred_fertility = 600,
   pickiness = 90,
}

wl.Descriptions():new_immovable_type{
   name = "palm_roystonea_desert_old",
   descname = _ "Roystonea regia Palm (Old)",
   species = _ "Roystonea regia Palm",
   icon = dirname .. "menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m50s",
         "transform=deadtree4 chance:15.23%",
         "seed=palm_roystonea_desert_sapling proximity:11.72%",
      },
      fall = {
         "remove=",
      },
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "old",
         fps = 10,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 24, 60 },
         sound_effect = {
            path = "sound/animals/crickets2",
            priority = "10%"
         },
      }
   },
}

pop_textdomain()

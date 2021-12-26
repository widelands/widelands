push_textdomain("world")

dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 150,
   preferred_humidity = 550,
   preferred_fertility = 500,
   pickiness = 80,
}

wl.Descriptions():new_immovable_type{
   name = "palm_oil_desert_old",
   descname = _ "Oil Palm (Old)",
   species = _ "Oil Palm",
   icon = dirname .. "menu.png",
   size = "small",

   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:13m20s",
         "transform=deadtree5 chance:19.53%",
         "seed=palm_oil_desert_sapling proximity:31.25%",
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

terrain_affinity = {
   -- Temperature is in arbitrary units.
   preferred_temperature = 125,

   -- Value between 0 and 1000 (1000 being very wet).
   preferred_humidity = 650,

   -- Values between 0 and 1000 (1000 being very fertile).
   preferred_fertility = 600,

   -- A value in [0, 100] that defines how well this can deal with non-ideal
   -- situations. Lower means it is less picky, i.e. it can deal better.
   pickiness = 60,
}

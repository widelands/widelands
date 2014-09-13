dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_fishing_rod",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fishing Rod",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fishing rods",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Fishing rods are needed by fishers, to catch fish. Produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}

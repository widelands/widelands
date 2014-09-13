dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_scythe",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Scythe",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"scythes",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The Scythe is the tool of the farmer. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 2 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wool",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wool",
   tribe = "empire",
   default_target_quantity = 10,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Wool is the hair of sheep. Weaving mills use it to make cloth.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 18 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_basket",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Basket",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"baskets",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This basket is needed by the vinefarmer for harvesting the grapes. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 11 },
      },
   }
}

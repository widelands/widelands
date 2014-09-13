dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_grape",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Grape",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"grapes",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 15 },
      },
   }
}

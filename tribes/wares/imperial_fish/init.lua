dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fish",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fish",
   tribe = "empire",
   default_target_quantity = 10,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Besides bread and meat, fish is also a foodstuff for the Empire. Fish are used in taverns, inns and training sites (arena, colosseum, training camp).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 6 },
      },
   }
}

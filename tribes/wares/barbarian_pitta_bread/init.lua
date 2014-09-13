dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_pitta_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Pitta Bread",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"pitta bread",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The barbarian bakers are best in making this flat and tasty pitta bread. It is made out of wheat and water following a secret recipe. Pitta bread is used in the taverns, inns and big inns to prepare rations, snacks and meals. It is also consumed at training sites (training camp and battle arena).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 11 },
      },
   }
}

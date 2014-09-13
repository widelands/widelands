dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fish",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fish",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Besides pitta bread and meat, fish is also a foodstuff for the barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 6 },
      },
   }
}

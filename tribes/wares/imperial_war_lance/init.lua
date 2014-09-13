dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_war_lance",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"War-Lance",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"war-lances",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is the best and sharpest weapon the Empire weaponsmith creates for the warriors. It is used in the training camp – together with food – to train soldiers from attack level 3 to level 4.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 3 },
      },
   }
}

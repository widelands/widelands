dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_stout",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Stout",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"stout",
   tribe = "barbarians",
   default_target_quantity = 15,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Only this beer is acceptable for the warriors in a battle arena. Some say that the whole power of the barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Stout is also used in big inns to prepare meals.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 8 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_hammer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hammer",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"hammers",
   tribe = "barbarians",
   default_target_quantity = 2,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The hammer is an essential tool. It is needed by the geologist, the builder, the blacksmith and the helmsmith. Make sure you always have some in reserve! It is one of the basic tools produced at the metal workshop, (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 4 },
      },
   }
}

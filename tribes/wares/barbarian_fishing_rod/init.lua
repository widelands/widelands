dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_fishing_rod",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fishing Rod",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fishing rods",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Fishing rods are needed by barbarian fishers for fishing. They are one of the basic tools produced in a metal workshop (but they cease to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}

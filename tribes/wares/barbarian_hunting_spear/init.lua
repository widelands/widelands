dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_hunting_spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hunting Spear",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"hunting spears",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters. Produced in the metal works (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill)",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 5 },
      },
   }
}

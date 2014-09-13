dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_fire_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fire Tongs",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fire tongs",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Fire tongs are tools of the smelter, who works in the smelting works. Produced by the metal works (but they cease to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 5 },
      },
   }
}

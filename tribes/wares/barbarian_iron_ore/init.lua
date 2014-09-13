dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_iron_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron Ore",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron ore",
   tribe = "barbarians",
   default_target_quantity = 15,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Iron ore is mined in iron mines. It delivers iron when smelted in a smelting works.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}

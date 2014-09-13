dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_iron_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron Ore",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron ore",
   tribe = "empire",
   default_target_quantity = 15,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Iron ore needs to be smelted in the smelting works to yield iron. Iron ore is produced in iron mines.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}

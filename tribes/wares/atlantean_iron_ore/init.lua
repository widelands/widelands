dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_iron_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron Ore",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron ore",
   tribe = "atlanteans",
   default_target_quantity = 15,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Iron ore is mined in iron mines. It is smelted in a smelting works to retrieve the iron.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}

dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_gold_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gold Ore",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"gold ore",
   tribe = "barbarians",
   default_target_quantity = 15,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Gold ore is mined in a goldmine. Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}

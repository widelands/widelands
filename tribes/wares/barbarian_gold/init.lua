dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_gold",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gold",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"gold",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Gold is the most valuable of all metals; only very important things are embellished with it. It is obtained out of gold ore in a smelting works. It is used as a precious building material and to produce different axes (in the war mill) and different parts of armor (in the helm smithy).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 10 },
      },
   }
}

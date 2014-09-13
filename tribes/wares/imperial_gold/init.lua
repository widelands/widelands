dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_gold",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gold",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"gold",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Gold is the most valuable of all metals; armor and weapons are embellished with it. It is produced in the smelting works out of gold ore and processed in the armor smithy and the weapon smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 10 },
      },
   }
}

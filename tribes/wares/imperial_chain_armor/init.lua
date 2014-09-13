dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_chain_armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Chain Armor",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"suits of chain armor",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The chain armor is a medium armor for Empire soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 11 },
      },
   }
}

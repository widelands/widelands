dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_battle_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Battle Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"battle axes",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is a dangerous weapon the barbarians are able to produce. It is produced in the ax factory. Only trained warriors are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}

-- ===================================
--    Campaign Specific Data
-- ===================================

return {
   barbarians = {
      workers_order = {
         {
            {
               name = "barbarians_file_animation",
               helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: File animation
               purpose = pgettext("barbarians_worker", "Carries items along your roads.")
            }
            }
         },
      },
      buildings = {
         {
            name = "barbarians_spritesheet_animation",
            helptexts = {
               -- TRANSLATORS: Lore helptext for a barbarian production site: Wood Hardener
               lore = pgettext("barbarians_building", [[‘Then he hit the door with his ax and, behold,<br>]] ..
                                             [[the door only trembled, but the shaft of the ax burst into a thousand pieces.’]]),
               -- TRANSLATORS: Lore author helptext for a barbarian production site: Wood Hardener
               lore_author = pgettext("barbarians_building", "Legend about a quarrel between the brothers Harthor and Manthor,<br>Manthor is said to be the inventor of blackwood"),
               -- TRANSLATORS: Purpose helptext for a barbarian production site: Wood Hardener
               purpose = pgettext("barbarians_building", "Hardens logs by fire into blackwood, which is used to construct robust buildings.")
            }
         }
      }
   }
}

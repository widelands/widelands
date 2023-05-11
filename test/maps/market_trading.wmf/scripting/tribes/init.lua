-- ===================================
--    Campaign Specific Data
-- ===================================

return {
   barbarians = {
      buildings = {
         {
            name = "barbarians_market",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for a Barbarian market: Market
               purpose = pgettext("barbarians_building", "A market enables trading with other tribes.")
            }
         },
         {
            name = "barbarians_custom_building",
            helptexts = {}
         }
      },
      workers_order = {
         {
            {
               name = "barbarians_custom_worker",
               helptexts = {
                  -- TRANSLATORS: Helptext for a Barbarian worker: Custom Worker
                  purpose = pgettext("barbarians_worker", "Custom Worker.")
               }
            }
         },
      },
      wares_order = {
         {
            {
               name= "custom_ware",
               default_target_quantity = 500,
               preciousness = 14,
               helptexts = {
                  purpose = pgettext("ware", "Custom ware does nothing")
               }
            },
            -- TODO(heesenfarmer): reinstate the possibility to replace a unit in scenarios
            -- {
               -- name= "ax_sharp",
               -- default_target_quantity = 1,
               -- preciousness = 1,
               -- helptexts = {
                  -- purpose = pgettext("custom_ware", "This is a replaced ware.")
               -- }
            -- }
         }
      }
   }
}

include "tribes/scripting/help/time_strings.lua"

push_textdomain("scenario_atl02.wmf")

local result = {
   atlanteans = {
      buildings = {
         {
            name = "atlanteans_trading_post",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean market site: Trading post
               purpose = pgettext("atlanteans_building", "Enables the share of goods with other players.")
            }
         },
         {
            name = "atlanteans_scouts_house1",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
               purpose = pgettext("atlanteans_building", "Explores unknown territory.")
            }
         },
      },
      workers_order = {
         {
            { name = "atlanteans_trader" }
         }
      },
      wares_order = {
         {
            {
               name= "coin_wood",
               preciousness = 1
            },
            {
               name= "coin_copper",
               preciousness = 1
            },
            {
               name= "coin_silver",
               preciousness = 1
            },
            {
               name= "coin_gold",
               preciousness = 1
            }
         },
      }
   },
   barbarians = {
      immovables = {
         {
            name = "barbarians_trade_pole",
            helptexts = {}
         },
      }
   }
}

pop_textdomain()
return result

include "tribes/scripting/help/time_strings.lua"

push_textdomain("scenario_atl02.wmf")

local result = {
   atlanteans = {
      buildings = {
         {
            name = "atlanteans_trading_post",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean market site: Trading post
               purpose = pgettext("building", "Enables the share of goods with other players.")
            }
         },
         {
            name = "atlanteans_scouts_house1",
            helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
            purpose = pgettext("building", "Explores unknown territory.")
            }
         },
      }
   }
}

pop_textdomain()
return result

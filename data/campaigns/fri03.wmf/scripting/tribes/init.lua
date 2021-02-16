include "tribes/scripting/help/time_strings.lua"

push_textdomain("scenario_fri03.wmf")

local result = {
   frisians = {
      buildings = {{
         name = "frisians_dikers_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Diker's House
            purpose = pgettext("frisians_building", "Constructs breakwaters nearby to gain new land from the sea.")
         }
      }},
      workers_order = {{{
         name = "frisians_diker",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian worker: Diker
            purpose = pgettext("frisians_worker", "Constructs breakwaters to gain new land from the sea.")
         }
      }}}
   },
   empire = {
      buildings = {{
         name = "empire_port_large",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Port
            purpose = pgettext("empire_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      }},
   }
}

pop_textdomain()
return result

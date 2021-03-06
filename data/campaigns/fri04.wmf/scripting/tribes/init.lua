push_textdomain("scenario_fri04.wmf")

local result = {
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

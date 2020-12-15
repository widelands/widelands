include "tribes/scripting/help/time_strings.lua"

push_textdomain("scenario_fri03.wmf")

local result = { frisians = {
   buildings = {{
      name = "frisians_debris_quarry",
      helptexts = {
         -- TRANSLATORS: Purpose helptext for a frisian production site: Debris Quarry
         purpose = pgettext("frisians_building", "Removes various obstacles from the ground.")
      }
   }},
   workers_order = {{{
      name = "frisians_debris_stonemason",
      helptexts = {
         -- TRANSLATORS: Purpose helptext for a frisian worker: Debris Stonemason
         purpose = pgettext("frisians_building", "Removes various obstacles from the ground.")
      }
   }}}
}}

pop_textdomain()
return result

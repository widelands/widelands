-- See data/campaigns/campaigns.lua and
-- https://www.widelands.org/documentation/add-ons/#campaign

--##########################################
--#      Campaign configuration - file     #
--##########################################

push_textdomain("_addon_", true)

local r = {
   --##########################################
   --#   Descriptions of difficulty levels    #
   --##########################################
   difficulties = {
      {
         -- This will be prefixed to any text that you might add in each
         -- campaign's difficulty description.
         descname = _"Easy.",
         -- An image to represent the difficulty level
         image = "images/ui_fsmenu/easy.png",
      },
   },

   --##########################################
   --#        The campaigns themselves        #
   --##########################################
   campaigns = {
      {
         descname = _"_descname_",
         tribe = "_tribe_",
         difficulty = { level=1, description=_"_descname_" },
         description = _"_description_",
         scenarios = {
            _scenarios_
            "dummy.wmf"
         }
      }
   }
}
pop_textdomain()
return r

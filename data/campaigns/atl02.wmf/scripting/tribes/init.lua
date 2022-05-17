include "tribes/scripting/help/time_strings.lua"

push_textdomain("scenario_atl02.wmf")

local result = {
   atlanteans = {
      buildings = {
         {
            name = "atlanteans_trading_post",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean market site: Trading post
               purpose = pgettext("atlanteans_building", "Allows sharing goods with other players.")
            }
         },
         {
            name = "atlanteans_scouts_house1",
            helptexts = {
               no_scouting_building_connected = pgettext("atlanteans_building", "You need to connect this flag to a scout’s house before you can send a scout here."),
               -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
               purpose = pgettext("building", "Explores unknown territory.")
            }
         },
         {
            name = "atlanteans_temple_of_satul",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean production site: Temple of Satul
               purpose = pgettext("atlanteans_building", "A big and expensive temple to worship Satul and consecrate priestesses.")
            }
         },
      },
      workers_order = {
         {
            { name = "atlanteans_trader",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean worker: Trader
               purpose = pgettext("atlanteans_worker", "Transports wares to share with other players."),
               note = pgettext("atlanteans_worker", "A trader is a scout who needs a horse to transport the wares.")
            }
            },
            { name = "atlanteans_initiate",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean worker: Satul's Initiate
               purpose = pgettext("atlanteans_worker", "Takes care of the Temple of Satul."),
               note = pgettext("atlanteans_worker", "Initiates take care of the Temple of Satul while they are learning to become priestesses.")
            }
            },
            { name = "atlanteans_priestess",
            helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean worker: Priestess
               purpose = pgettext("atlanteans_worker", "Worships Satul."),
               note = pgettext("atlanteans_worker", "Priestesses are needed to make Satul show his might.")
            }
            }
         }
      },
      wares_order = {
         {
            {
               name= "coin_wood",
               default_target_quantity = 5,
               preciousness = 1,
               helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean ware: Wooden Coin
               purpose = pgettext("atlanteans_ware", "A wooden coin, received as reward for delivering five logs.")
               }
            },
            {
               name= "coin_copper",
               default_target_quantity = 4,
               preciousness = 1,
               helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean ware: Copper Coin
               purpose = pgettext("atlanteans_building", "A coin made of copper, received as reward for delivering four lumps of coal.")
               }
            },
            {
               name= "coin_silver",
               default_target_quantity = 3,
               preciousness = 1,
               helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean ware: Silver Coin
               purpose = pgettext("atlanteans_building", "A silver coin, received as reward for delivering three iron ingots.")
               }
            },
            {
               name= "coin_gold",
               default_target_quantity = 2,
               preciousness = 1,
               helptexts = {
               -- TRANSLATORS: Purpose helptext for an atlantean ware: Golden Coin
               purpose = pgettext("atlanteans_building", "A golden coin, received as reward for delivering two bars of gold.")
               }
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

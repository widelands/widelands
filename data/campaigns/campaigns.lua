--##########################################
--#      Campaign configuration - file     #
--##########################################

return {
   campaigns = {
      {
         -- Barbarians Introduction
         -- The name the user sees on screen
         descname = _"The Second Empire",
         -- Internal name for reference
         name = "barbarians",
         -- The tribe that the user will be playing as displayed on screen
         tribe = _"Barbarians",
         -- The difficulty of this campaign. Value range is 1-3 (easy-hard), or 0 for "not set"
         difficulty = { value=1, description=_"Easy. Introduces the Barbarians" },
         -- An introduction story
         description = _"When Chat’Karuth died, he was an old man, father to three strong and ambitious sons, and warlord to an army that could match any enemy willing to rise against the ancient forests. Though at the end of his glorious reign, Chat’Karuth chose his eldest son, Thron, to succeed him as the tribe’s warlord – a decision that left his two brothers unsatisfied. The old warlord knew that. As his father instructed him, Thron left the capital of Al’thunran, the home of the Throne Among the Trees, and withdrew his forces to the high hills where he buried the corpse of his father. There he swore to the gods and his father’s spirit that he’d return to re-established order. While his brothers have raged blind war against Thron and the few forces he left to secure the borders of Al’thunran, the young warlord seeks to reunite his ambitious brothers and force the tribes to march once again under a common banner.",
         -- The campaign's scenarios. The first scenario is always visible if
         -- the campaign itself is visible, the following scenarios as well as
         -- campaigns marked as `visible = false` need to be unlocked with
         -- `reveal_scenario`/`reveal_campaign` on successful completion.
         -- Call `player:mark_as_solved` in your scenario code when a user has
         -- solved a scenario
         scenarios = {
            {
               -- The name the user sees on screen
               descname = _"A Place to Call Home",
               -- Internal name for reference
               name = "bar01",
               -- Path to the scenario, relative to data/campaigns
               path = "bar01.wmf",
               -- The scenario to reveal when `player:mark_as_solved` is called
               reveal_scenario = "bar02",
            },
            {
               descname = _"This Land is Our Land",
               name = "bar02",
               path = "bar02.wmf",
               -- The campaign to reveal when player:mark_as_solved is called
               reveal_campaign = "empire",
            }
         }
      },
      {
         -- Empire Introduction
         descname = _"The Months of Exile",
         name = "empire",
         tribe = _"Empire",
         difficulty = { value=1, description=_"Easy. Introduces the Empire" },
         description = _"Six months ago, Lutius – a young general of the Empire – was sent with 150 soldiers to the frontier beyond the northern forests where Barbarian tribes were crossing onto land held by the Empire. His task was to defend the Empire’s land. At first, everything was calm. He even talked to a few Barbarian children and thought about a peaceful life – side by side with this archaic folk. He began to feel safer and his army began to drop their attention off the potential enemy. That was their undoing. One night in March his unprepared army was attacked by 100 Barbarian footmen and was completely scattered. Only with his bare life he and a handful of his soldiers survived.",
         -- If `prerequisite` is present, the campaign is greyed out by default.
         -- The campaign name reference is used to show to the user which campaign
         -- to play in order to unlock this campaign.
         prerequisite = "barbarians",
         -- For scenarios that might have been added to Widelands since the last time that the user played a campaign
         reveal_scenarios = { "bar02" },
         scenarios = {
            {
               descname = _"The Strands of Malac’ Mor",
               name = "emp01",
               path = "emp01.wmf",
               reveal_scenario = "emp02",
            },
            {
               descname = _"An Outpost for Exile",
               name = "emp02",
               path = "emp02.wmf",
               reveal_scenario = "emp03",
            },
            {
               descname = _"Neptune’s Revenge",
               name = "emp03",
               path = "emp03.wmf",
               reveal_scenario = "emp04",
               reveal_campaign = "atlanteans",
            },
            {
               descname = _"Not yet implemented",
               name = "emp04",
               path = "dummy.wmf",
               reveal_campaign = "atlanteans",
            },
         }
      },
      {
         -- Atlantean Introduction
         descname = _"The Run for the Fire",
         name = "atlanteans",
         tribe = _"Atlanteans",
         difficulty = { value=2, description=_"Challenging. Introduces the Atlanteans" },
         description = _"When their God lost faith in the Atlanteans and drowned their island, one woman’s struggle for justice and a second chance for her people would become the stuff of legends. Leading the remaining Atlanteans into a new future in a new part of the World, Jundlina became the most powerful human of her time, but at a high cost: her humanity and soul.",
         prerequisite = "empire",
         reveal_scenarios = { "emp02", "emp03" },
         scenarios = {
            {
               descname = _"From Nemesis to Genesis",
               name = "atl01",
               path = "atl01.wmf",
               reveal_scenario = "atl02",
            },
            {
               descname = _"Not yet implemented",
               name = "atl02",
               path = "dummy.wmf"
            },
         }
      }
   }
}

--##########################################
--#     Tutorials configuration - file     #
--##########################################

return {
	campaigns = {
		{
			-- Dummy reference to satisfy the format
			name = "tutorials",
			-- The tutorial scenarios in the order that they will appear on screen
			scenarios = {
				{
					-- The name the user sees on screen
					descname = _"Basic Control",
					-- Internal name for reference
					name = "basic",
					-- Path to the scenario, relative to data/campaigns
					path = "tutorial01_basic_control.wmf"
				},
				{
					descname = _"Warfare",
					name = "warfare",
					path = "tutorial02_warfare.wmf"
				},
				{
					descname = _"Seafaring",
					name = "seafaring",
					path = "tutorial03_seafaring.wmf"
				},
				{
					descname = _"Economy",
					name = "economy",
					path = "tutorial04_economy.wmf"
				}
			}
		}
	}
}

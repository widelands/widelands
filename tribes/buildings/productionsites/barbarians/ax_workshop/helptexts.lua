function building_helptext_lore()
	-- TRANSLATORS: Lore helptext for a building
	return pgettext("barbarians_building", "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’")
end

function building_helptext_lore_author()
	-- TRANSLATORS: Lore author helptext for a building
	return pgettext("barbarians_building", "An old Barbarian proverb<br> meaning that you need to take some risks sometimes.")
end

function building_helptext_purpose()
	-- TRANSLATORS: Purpose helptext for a building
	return pgettext("barbarians_building", "Produces axes, sharp axes and broad axes.")
end

function building_helptext_note()
	-- TRANSLATORS: Note helptext for a building
	return pgettext("barbarians_building", "The barbarian ax workshop is the intermediate production site in a series of three buildings. It is an upgrade from the metal workshop but doesn’t require additional qualification for the worker.")
end

function building_helptext_performance()
	-- TRANSLATORS: Performance helptext for a building
	return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)) .. " " .. pgettext("barbarians_building", "All three weapons take the same time for making, but the required raw materials vary.")
end

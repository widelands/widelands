function building_helptext_lore()
	-- TRANSLATORS: Lore helptext for a building
	return pgettext("barbarians_building", [[‘He loves the animals and to breed them<br>
			as we love to cook and eat them.’]])
end

function building_helptext_lore_author()
	-- TRANSLATORS: Lore author helptext for a building
	return pgettext("barbarians_building", "Barbarian nursery rhyme")
end

function building_helptext_purpose()
	-- TRANSLATORS: Purpose helptext for a building
	return pgettext("barbarians_building", "Releases animals into the wild to steady the meat production.")
end

function building_helptext_note()
	-- #TRANSLATORS: Note helptext for a building
	return ""
end

function building_helptext_performance()
	-- TRANSLATORS: Performance helptext for a building
	return pgettext("barbarians_building", "The gamekeeper pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 52.5):bformat(52.5))
end

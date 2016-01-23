function building_helptext_lore()
   --"Wer zwei Halme dort wachsen lässt, wo sonst nur einer wächst, der ist größer als der größte Feldherr!" – Friedrich der Große
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘He who can grow two trees where normally only one will grow exceeds the most important general!’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Chat’Karuth in a conversation with a Ranger")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Plants trees in the surrounding area.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The ranger’s hut needs free space within the work area to plant the trees.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "The ranger needs %s to plant a tree, not counting the time he needs to reach the destination and go home again."):bformat(ngettext("%d second", "%d seconds", 5):bformat(5))
end

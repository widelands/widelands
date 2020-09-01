function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", [[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", [[Song written by Sigurd the Bard when the first rain fell after the Great Drought in the 21ˢᵗ year of Chat’Karuth’s reign.]])
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Draws water out of the deep.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "The carrier needs %s to get one bucket full of water."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
end

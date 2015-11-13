function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", [[‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity ...’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", [[Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.]])
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore."):bformat("2/3")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If the food supply is steady, this mine can produce gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 19.5):bformat(19.5))
end

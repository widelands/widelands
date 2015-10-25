function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		default = pgettext("ware", "Fire tongs are the tools of the smelter, who works in the smelting works."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		atlanteans = pgettext("atlanteans_ware", "They are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		barbarians = pgettext("barbarians_ware", "Produced by the metal works (but they cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		empire = pgettext("empire_ware", "They are produced by the toolsmith.")
   }
   local result = ""
   if tribe then
		result = helptext[tribe]
	else
		result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end

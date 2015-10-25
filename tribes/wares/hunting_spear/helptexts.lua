function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
		default = pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
		barbarians = pgettext("barbarians_ware", "Hunting spears are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
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

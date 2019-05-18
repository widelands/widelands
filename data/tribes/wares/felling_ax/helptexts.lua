function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Felling Ax
      default = pgettext("ware", "The felling ax is the tool to chop down trees."),
      -- TRANSLATORS: Helptext for a ware: Felling Ax
      barbarians = pgettext("barbarians_ware", "Felling axes are used by lumberjacks and produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Felling Ax
      frisians = pgettext("frisians_ware", "Felling axes are used by woodcutters and produced by the blacksmithy."),
      -- TRANSLATORS: Helptext for a ware: Felling Ax
      empire = pgettext("empire_ware", "Felling axes are used by lumberjacks and produced by the toolsmithy.")
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

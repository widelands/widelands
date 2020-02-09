function immovable_helptext(tribe)
   local helptext = {
      atlanteans = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for an Atlantean resource: Stones
         _("Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine. You will also get granite from the mine."),
         -- TRANSLATORS: Helptext for an Atlantean resource: Stones
         _("There are only a few precious stones here.")),
      barbarians = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for a Barbarian resource: Stones
         _("Granite is a basic building material and can be dug up by a granite mine."),
         -- TRANSLATORS: Helptext for a Barbarian resource: Stones
         _("There is only a little bit of granite here.")),
      empire = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for an Empire resource: Stones
         _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
         -- TRANSLATORS: Helptext for an Empire resource: Stones
         _("There is only a little bit of marble here.")),
      frisians = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for a Frisian resource: Stones
         _("Granite is a basic building material and can be dug up by a rock mine."),
         -- TRANSLATORS: Helptext for a Frisian resource: Stones
         _("There is only a little bit of granite here.")),
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

-- This file defines the font sets available for different languages.
--
-- Define which fontset you wish to use for your language in locales.lua.
--
-- When adding a new fontset, also make sure you have all pertinent license
-- and source information collected in its folder.


return {
   -- This is the default set and always needs to be complete. It covers all possible font styles.
   default = {
      -- If your language doesn't distinguish between serif and sans serif fonts, please use the sans set.
      -- Your font set should always define "sans".
      sans = "DejaVu/DejaVuSans.ttf",
      sans_bold = "DejaVu/DejaVuSans-Bold.ttf",
      sans_italic = "DejaVu/DejaVuSans-Oblique.ttf",
      sans_bold_italic = "DejaVu/DejaVuSans-BoldOblique.ttf",
      -- If your font set doesn't have serif or condensed variants, sans will be used instead.
      serif = "DejaVu/DejaVuSerif.ttf",
      -- Add bold and italic variants if your font set has them.
      serif_bold = "DejaVu/DejaVuSerif-Bold.ttf",
      serif_italic = "DejaVu/DejaVuSerif-Italic.ttf",
      serif_bold_italic = "DejaVu/DejaVuSerif-BoldItalic.ttf",
      condensed = "DejaVu/DejaVuSansCondensed.ttf",
      condensed_bold = "DejaVu/DejaVuSansCondensed-Bold.ttf",
      condensed_italic = "DejaVu/DejaVuSansCondensed-Oblique.ttf",
      condensed_bold_italic = "DejaVu/DejaVuSansCondensed-BoldOblique.ttf",
      -- If the direction isn't defined, your font set will default to "ltr" = left-to-right.
      -- For right-to-left or bidirectional (BiDi) languages, use "rtl".
      direction = "ltr",
      -- This is used to calculate line height for the fontset. Use any character for your language's script.
      representative_character = "a",
      size_offset = 0
   },

   arabic = {
      sans = "amiri/amiri-regular.ttf",
      sans_bold = "amiri/amiri-bold.ttf",
      sans_italic = "amiri/amiri-slanted.ttf",
      sans_bold_italic = "amiri/amiri-boldslanted.ttf",
      direction = "rtl",
      representative_character = "ا",
      size_offset = 4
   },

   cjk = {
      sans = "MicroHei/wqy-microhei.ttc",
      representative_character = "简",
   },

   devanagari = {
      sans = "Nakula/nakula.ttf",
      representative_character = "ह",
      direction = "rtl",
      size_offset = 2
   },

   hebrew = {
      sans = "Culmus/TaameyFrankCLM-Medium.ttf",
      sans_bold = "TaameyFrankCLM-Bold.ttf",
      sans_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
      serif = "Culmus/TaameyFrankCLM-Medium.ttf",
      serif_bold = "TaameyFrankCLM-Bold.ttf",
      serif_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
      representative_character = "א",
      direction = "rtl",
      size_offset = 4
   },

   myanmar = {
      sans = "mmrCensus/mmrCensus.ttf",
      representative_character = "မ",
      size_offset = 2
   },

   sinhala = {
      sans = "Sinhala/lklug.ttf",
      representative_character = "ස",
   }
}

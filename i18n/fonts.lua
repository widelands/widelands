-- This file defines the font sets available for different languages.
--
-- Define which fontset you wish to use for your language in locales.lua.
--
-- The "default" font set covers all possible font styles.
--
-- Your font might not have all of these available. In this case, it is
-- sufficient to define "serif" at the least, a fallback scheme is in place.
-- But do define everything your fontset has.
--
-- When adding a new fontset, also make sure you have all pertinent license
-- and source information collected in its folder.


return {
   -- This is the default set and always needs to be complete.
	default = {
		serif = "DejaVu/DejaVuSerif.ttf",
		serif_bold = "DejaVu/DejaVuSerif-Bold.ttf",
		serif_italic = "DejaVu/DejaVuSerif-Italic.ttf",
		serif_bold_italic = "DejaVu/DejaVuSerif-BoldItalic.ttf",
		sans = "DejaVu/DejaVuSans.ttf",
		sans_bold = "DejaVu/DejaVuSans-Bold.ttf",
		sans_italic = "DejaVu/DejaVuSans-Oblique.ttf",
		sans_bold_italic = "DejaVu/DejaVuSans-BoldOblique.ttf",
		condensed = "DejaVu/DejaVuSansCondensed.ttf",
		condensed_bold = "DejaVu/DejaVuSansCondensed-Bold.ttf",
		condensed_italic = "DejaVu/DejaVuSansCondensed-Oblique.ttf",
		condensed_bold_italic = "DejaVu/DejaVuSansCondensed-BoldOblique.ttf",
		direction = "ltr"
	},

	arabic = {
		serif = "FaKacstBook/FaKacstBook.ttf",
		direction = "rtl"
	},

	cjk = {
		serif = "MicroHei/wqy-microhei.ttc",
	},

	devanagari = {
		serif = "Nakula/nakula.ttf",
		direction = "rtl"
	},

	hebrew = {
		serif = "Culmus/TaameyFrankCLM-Medium.ttf",
		serif_bold = "TaameyFrankCLM-Bold.ttf",
		serif_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
		sans = "Culmus/TaameyFrankCLM-Medium.ttf",
		sans_bold = "TaameyFrankCLM-Bold.ttf",
		sans_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
		direction = "rtl"
	},

	myanmar = {
		serif = "mmrCensus/mmrCensus.ttf",
	},

	sinhala = {
		serif = "Sinhala/lklug.ttf",
	}
}

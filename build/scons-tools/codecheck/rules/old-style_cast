#!/usr/bin/python


"""
This catches some old-style casts, that should be caught with -Wold-style-cast,
and when gcc-4.2 is common, -Werror=old-style-cast. It is complex because it
should allow "operator==(h)" and "operator!=(h)". It should also ignore macros.
"""

error_msg = "Old C-Style cast. Change to static_cast<Type>(var) or similar!"

strip_comments_and_strings = True
strip_macros = True
regexp="""(?x)
(((((((((([_a-zA-Z0-9]o|[^o]|^)p|[^p]|^)e|[^e]|^)r|[^r]|^)a|[^a]|^)t|[^t]|^)o|[^o]|^)r|[^r]|^)[!=]|[^!=]|^)=|[^] "'_a-zA-Z0-9>=]|return|^)
[ ]*
\([ ]*((const|enum|long|(un)?signed|short|struct)[ ]+)*
([_a-zA-Z][_a-zA-Z0-9]*[ ]*::)*[ ]*[_a-zA-Z][_a-zA-Z0-9]*[ ]*
((const)?[ ]*\*[ ]*)*
(const *)?
\)[ ]*
([^].:,;{)=<>|?! c*/%+\-\n]|c([^o]|o([^n]|n([^s]|s([^t]|t[_a-zA-Z0-9]))))
)"""


forbidden = [
    '(a)b',
    '(X::a)b',
    '(X ::a)b',
    '(X    ::a)b',
    '(X:: a)b',
    '(X::    a)b',
    '(a *)b',
    '(a const *)b',
    '(a **)b',
    '(a * * )b',
    '(a * * const)b',
    '(a const * * const)b',
    '=(a) b',
    'goperator!=(b) d',
    'goperator==(b) r',
    'operagor!=(b) d',
    'operagor==(b) r',
    'perator!=(b) d',
    'perator==(b) r',
    'return (PIL*)odr;',
    '(char*)&v',
    '	(enum arb) agr',
    '		(struct arb) agr',
    '			(const arb) agr',
    '(I const) a;',
    '(I const) ca;',
    '(I const) coa;',
    '(I const) cona;',
    '(I const) consa;',
    '(I const) consta;',
    '(I)a;',
    '(I)ca;',
    '(I)coa;',
    '(I)cona;',
    '(I)consa;',
    '(I)consta;',
]

allowed = [
    'a(b);',
    'operator==(b) and',
    'operator!=(b) or',
    '(I const) const;',
    '(I const) const',
    '(I)const',
    "(dynamic_cast<ConstructionSite const *>(bld) or",
	"void set_defeating_player(const Player_Number player_number) throw ()",
	"virtual WaresQueue & waresqueue(Ware_Index) __attribute__ ((noreturn));",
    "get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL));",
	"if (not er)                          er = &m_table.add(s);",
	"if (m_linknext) m_linknext->m_linkpprev = &m_linknext;",
	
    "\t(Widelands::Building_Index::value_t)\n",
]

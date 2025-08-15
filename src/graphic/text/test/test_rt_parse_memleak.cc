/*
 * Copyright (C) 2007-2025 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

// #include <gperftools/heap-checker.h>  // is outdated, see below
#include <memory>

#include "base/test.h"
// #include "graphic/text/rt_errors_impl.h"  // for RT::EndOfText
#include "graphic/text/rt_parse.h"
#include "graphic/text/textstream.h"

/******************/
/* Helper classes */
/******************/

/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
// struct WlTestFixture { ... }

TESTSUITE_START(RtParseMemleak)

/*
 * Simple tests
 */

TESTCASE(parser_parse) {
	RT::Parser parser;
	// std::unique_ptr<RT::Parser()> parser(new RT::Parser());  // wrong, but similar would be better
	RT::TagSet allowed_tags = RT::TagSet();
#ifdef BASE_HEAP_CHECKER_H_
	// HeapLeakChecker became a dummy only since gperftools-2.16.90, TODO(somebody): replace with ...
	HeapLeakChecker heap_checker("test_parse");
#endif
	// leak from Parser::parse()
	{
		try {
			parser.parse("<a", allowed_tags);
		/* } catch (RT::EndOfText) {  // reports compile error
			// expected
			// */
		} catch (std::exception& e) {
			/* if (typeid(e).name().find("EndOfText") < 0) {  // is char* and not std::string
				throw;  // unexpected, throw it again
			}  // else expected
			// */
		}
	}
#ifdef BASE_HEAP_CHECKER_H_
	check_equal(heap_checker.NoLeaks(), true);
#endif
	// triggering more leaks
	{
		try {
			parser.parse("<rt><p>Title <not_a_tag> more&nbsp;text</p></rt>", allowed_tags);
		} catch (std::exception& e) {
			std::cout << "DEBUG exception: " << typeid(e).name() << "\n";
		}
	}
}

TESTSUITE_END()

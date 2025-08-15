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

#ifdef BASE_HEAP_CHECKER_H_
// HeapLeakChecker became a dummy only since gperftools-2.16.90, TODO(somebody): replace with ...
#define init_memory_block_vars() HeapLeakChecker heap_checker;
#define start_memory_block(block_name) heap_checker = HeapLeakChecker(block_name)
#define end_memory_block() check_equal(heap_checker.NoLeaks(), true)
#else
#define init_memory_block_vars()
#define start_memory_block(block_name)
#define end_memory_block()
#endif

/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
// struct WlTestFixture { ... }

TESTSUITE_START(RtParseMemleak)

/*
 * Simple tests
 */

TESTCASE(parser_parse) {
	init_memory_block_vars();
	RT::Parser parser;
	// std::unique_ptr<RT::Parser()> parser(new RT::Parser());  // wrong, but similar would be better
	RT::TagSet allowed_tags = RT::TagSet();
	// normal parse
	{
		std::unique_ptr<RT::Tag> tag(parser.parse("<rt><p>some text</p></rt>", allowed_tags));
		check_equal(tag->name(), "rt");
		check_equal(tag->children().size(), 1);
		check_equal(tag->children()[0]->tag ? "" : "tag is nullptr", "");
		check_equal(tag->children()[0]->tag->name(), "p");
		check_equal(tag->children()[0]->tag->children().size(), 1);
		check_equal(tag->children()[0]->tag->children()[0]->text, "some text");
	}
	// leak from Parser::parse()
	start_memory_block("test_parse");
	{
		try {
			parser.parse("<a", allowed_tags);
			check_equal("no error raised", "");
		/* } catch (RT::EndOfText) {  // reports compile error
			// */
		} catch (std::exception& e) {
			/* if (typeid(e).name().find("EndOfText") < 0) {  // is char* and not std::string
				throw;  // unexpected, throw it again
			}  // TODO(somebody): check here or above */
		}
	}
	end_memory_block();
	// triggering more leaks
	start_memory_block("more leaks");
	{
		try {
			parser.parse("<rt><p>Title <not_a_tag> more&nbsp;text</p></rt>", allowed_tags);
			check_equal("no error raised", "");
		/* } catch (RT::SyntaxError) {  // reports compile error
			// */
		} catch (std::exception& e) {
			/* Fails, names are different
			if (typeid(e).name() != typeid(RT::EndOfText).name()) {
				throw;  // unexpected, throw it again
			}  // TODO(somebody): check here or above */
		}
	}
	end_memory_block();
}

TESTSUITE_END()

/*
 * Copyright (C) 2007-2013 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

//  FIXME remove this pragma when boost-1.40 is required (when it has been
//  FIXME accepted by distributions)
#pragma GCC diagnostic ignored "-Wold-style-cast"

#include <exception>

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>

#include "container_iterate.h"
#include "economy/flag.h"
#include "economy/iroute.h"
#include "economy/itransport_cost_calculator.h"
#include "economy/router.h"
#include "economy/routing_node.h"
#include "logic/wareworker.h"
#include "logic/widelands_geometry.h"

using namespace Widelands;

/******************/
/* Helper classes */
/******************/
/// Helper classes {{{
class BadAccess : public std::exception {};
class TestingRoutingNode : public RoutingNode {
public:
	TestingRoutingNode
		(int32_t wcost = 0, Coords pos = Coords(0, 0)) :
			_waitcost(wcost), _position(pos) {}
	void add_neighbour(TestingRoutingNode * nb) {
		_neighbours.push_back(nb);
	}
	TestingRoutingNode * get_neighbour(uint8_t idx) const {
		if (idx >= _neighbours.size())
			throw BadAccess();
		return _neighbours[idx];
	}

	virtual Flag & base_flag() override {return _flag;}
	void set_waitcost(int32_t const wc) {_waitcost = wc;}
	int32_t get_waitcost() const {return _waitcost;}
	const Coords & get_position() const override {return _position;}

	void get_neighbours(WareWorker type, RoutingNodeNeighbours &) override;

	// test functionality
	bool all_members_zeroed();

private:
	typedef std::vector<TestingRoutingNode *> Neigbours;

	Neigbours _neighbours;
	int32_t _waitcost;
	Coords _position;
	Flag _flag;
};
void TestingRoutingNode::get_neighbours(WareWorker type, RoutingNodeNeighbours & n) {
	container_iterate_const(Neigbours, _neighbours, i)
		// second parameter is walktime in ms from this flag to the neighbour.
		// only depends on slope
		n.push_back(RoutingNodeNeighbour(*i.current, 1000 * ((type == wwWARE)?1 + _waitcost:1)));
}
bool TestingRoutingNode::all_members_zeroed() {
	bool integers_zero =
		!mpf_cycle &&  !mpf_realcost && !mpf_estimate;
	bool pointers_zero = (mpf_backlink == nullptr);

	return pointers_zero && integers_zero;
}

class TestingTransportCostCalculator : public ITransportCostCalculator {
	int32_t calc_cost_estimate(Coords c1, Coords c2) const override {
		// We use an euclidian metric here. It is much easier for
		// test cases
		double xd = (c1.x - c2.x);
		double yd = (c1.y - c2.y);
		return static_cast<int32_t>((xd * xd + yd * yd) * 1000);
	}
};
class TestingRoute : public IRoute {
public:
	typedef std::vector<RoutingNode *> Nodes;

	void init(int32_t) override {
		nodes.clear();
	}
	void insert_as_first(RoutingNode * node) override {
		nodes.insert(nodes.begin(), node);
	}

	int32_t get_length() {return nodes.size();}

	bool has_node(RoutingNode * const n) {
		container_iterate_const(Nodes, nodes, i)
			if (*i.current == n)
				return true;
		return false;
	}
	bool has_chain(Nodes & n) {
		bool chain_begin_found = false;

		Nodes::iterator j = n.begin();
		Nodes::iterator i = nodes.begin();
		while (i != nodes.end() && j != n.end()) {
			if (!chain_begin_found) {
				if (*i == *j) {
					chain_begin_found = true;
					++j;
				}
				if (j == n.end())
					return true;
			} else {
				if (*i != *j) {
					j = n.begin();
					chain_begin_found = false;
					if (*i == *j) {
						chain_begin_found = true;
						++j;
					}
				} else {
					++j;
					if (j == n.end()) {
						return true;
					}
				}
			}
			++i;
		}

		return false;
	}
private:
	Nodes nodes;
};

/// End of helper classes }}}


/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
BOOST_AUTO_TEST_SUITE(Routing)

// {{{ TestingRoutingNode Test Cases
/*
 * First test the TestingRoutingNode class, so that we
 * are sure it works in the other tests
 *
 */
BOOST_AUTO_TEST_CASE(testingnode_creation) {
	TestingRoutingNode d0;
	TestingRoutingNode d1(0, Coords(15, 0));

	BOOST_CHECK_EQUAL(d0.get_position().y, d1.get_position().y);
	BOOST_CHECK_EQUAL(d0.get_position().x, 0);
	BOOST_CHECK_EQUAL(d1.get_position().x, 15);
}
struct TestingNode_DefaultNodes_Fixture {
	TestingNode_DefaultNodes_Fixture() {
		d0 = new TestingRoutingNode();
		d1 = new TestingRoutingNode(1, Coords(15, 0));
		nodes.push_back(d0);
		nodes.push_back(d1);
	}
	~TestingNode_DefaultNodes_Fixture() {
		while (!nodes.empty()) {
			TestingRoutingNode * n = nodes.back();
			delete n;
			nodes.pop_back();
		}
	}
	std::vector<TestingRoutingNode *> nodes;
	TestingRoutingNode * d0;
	TestingRoutingNode * d1;
};
BOOST_FIXTURE_TEST_CASE
	(testingnode_neighbour_attaching, TestingNode_DefaultNodes_Fixture)
{
	d0->add_neighbour(d1);

	BOOST_CHECK_EQUAL(d0->get_neighbour(0), d1);
}
BOOST_FIXTURE_TEST_CASE
	(testingnode_illegalneighbour_access, TestingNode_DefaultNodes_Fixture)
{
	try {
		d0->get_neighbour(0);
		BOOST_ERROR("BadAccess not thrown");
	} catch (BadAccess &) {
	}
}
// }}} End of TestingRoutingNode Test cases

// {{{ RoutingNode Tests
/*
 * Now test the routing nodes functionality
 */
BOOST_AUTO_TEST_CASE(RoutingNode_InitializeMemberVariables) {
	TestingRoutingNode d0(0, Coords(15, 0));

	BOOST_CHECK(d0.all_members_zeroed());
}

struct SimpleRouterFixture {
	SimpleRouterFixture() : r(boost::bind(&SimpleRouterFixture::reset, this)) {
		d0 = new TestingRoutingNode();
		d1 = new TestingRoutingNode(1, Coords(15, 0));
		vec.push_back(d0);
		vec.push_back(d1);
	}
	~SimpleRouterFixture() {
		delete d0;
		delete d1;
	}
	/**
	 * Callback for the incredibly rare case that the \ref Router pathfinding
	 * cycle wraps around.
	 */
	void reset() {
		if (d0) d0->reset_path_finding_cycle();
		if (d1) d1->reset_path_finding_cycle();
	}
	TestingRoutingNode * d0;
	TestingRoutingNode * d1;
	std::vector<RoutingNode *> vec;
	Router r;
	TestingRoute route;
	TestingTransportCostCalculator cc;
};


// {{{ TestingRoute Testing
BOOST_AUTO_TEST_CASE(TestingRoute_emptyatstart) {
	TestingRoute r;
	BOOST_CHECK_EQUAL(r.get_length(), 0);
}
BOOST_AUTO_TEST_CASE(TestingRoute_addnode) {
	TestingRoute r;
	TestingRoutingNode d;
	r.insert_as_first(&d);

	BOOST_CHECK_EQUAL(r.get_length(), 1);
}
BOOST_FIXTURE_TEST_CASE(TestingRoute_hasnode, SimpleRouterFixture) {
	BOOST_CHECK_EQUAL(route.has_node(d0), false);
	BOOST_CHECK_EQUAL(route.has_node(d1), false);
	route.insert_as_first(d0);
	BOOST_CHECK_EQUAL(route.has_node(d0), true);
	BOOST_CHECK_EQUAL(route.has_node(d1), false);
	route.insert_as_first(d1);
	BOOST_CHECK_EQUAL(route.has_node(d0), true);
	BOOST_CHECK_EQUAL(route.has_node(d1), true);
}
BOOST_FIXTURE_TEST_CASE(TestingRoute_haschain, SimpleRouterFixture) {
	std::vector<RoutingNode *> chain;

	chain.push_back(d0);
	route.insert_as_first(d0);
	BOOST_CHECK_EQUAL(route.has_chain(chain), true);

	route.insert_as_first(d1);
	chain.insert(chain.begin(), d1);
	BOOST_CHECK_EQUAL(route.has_chain(chain), true);
}
BOOST_FIXTURE_TEST_CASE
	(TestingRoute_chainisunidirectional, SimpleRouterFixture)
{
	std::vector<RoutingNode *> chain;
	// Chains are unidirectional. Check that chain.clear();
	chain.push_back(d0);
	chain.push_back(d1);
	route.insert_as_first(d0);
	route.insert_as_first(d1);
	BOOST_CHECK_EQUAL(route.has_chain(chain), false);
}
BOOST_FIXTURE_TEST_CASE
	(TestingRoute_haschain_checksubchain, SimpleRouterFixture)
{
	// Do not get confused when a partial chain is found
	TestingRoutingNode d;
	std::vector<RoutingNode *> chain;

	chain.push_back(&d);
	chain.push_back(d0);
	chain.push_back(d1);

	route.insert_as_first(&d);
	route.insert_as_first(d0);
	route.insert_as_first(&d); // d -> d0 -> d
	BOOST_CHECK_EQUAL(route.has_chain(chain), false);
	route.insert_as_first(d1);
	route.insert_as_first(d0);
	route.insert_as_first(&d);
	route.insert_as_first(d0);
	route.insert_as_first(d1); // d1 d0 d d0 d1 d d0 d
	BOOST_CHECK_EQUAL(route.has_chain(chain), true);
}
BOOST_FIXTURE_TEST_CASE
	(TestingRoute_haschain_checksubchain_endisnotstart, SimpleRouterFixture)
{
	// Do not get confused when a partial chain is found
	TestingRoutingNode d;
	std::vector<RoutingNode *> chain;

	chain.push_back(&d);
	chain.push_back(d0);
	chain.push_back(d1);

	route.insert_as_first(&d);
	route.insert_as_first(d0);
	route.insert_as_first(d0);
	BOOST_CHECK_EQUAL(route.has_chain(chain), false);
	route.insert_as_first(d0);
	route.insert_as_first(d0);
	route.insert_as_first(&d);
	route.insert_as_first(d0);
	BOOST_CHECK_EQUAL(route.has_chain(chain), false);
	route.insert_as_first(d1);
	route.insert_as_first(d0);
	route.insert_as_first(&d);
	route.insert_as_first(d0);
	route.insert_as_first(d1);
	BOOST_CHECK_EQUAL(route.has_chain(chain), true);
}
BOOST_FIXTURE_TEST_CASE(TestingRoute_init, SimpleRouterFixture) {
	route.insert_as_first(d0);
	BOOST_CHECK_EQUAL(route.get_length(), 1);
	route.init(0);
	BOOST_CHECK_EQUAL(route.get_length(), 0);
	route.insert_as_first(d1);
	BOOST_CHECK_EQUAL(route.get_length(), 1);
}
// }}}

// {{{ Router Test-Cases
/*************************************************************************/
/*                           EQUAL COST TESTS                            */
/*************************************************************************/
/****************/
/* SIMPLE TESTS */
/****************/
BOOST_FIXTURE_TEST_CASE
	(router_findroute_seperatedNodes_exceptFail, SimpleRouterFixture)
{
	bool rval = r.find_route
		(*d0, *d1,
		 &route,
		 wwWORKER,
		 -1,
		 cc);

	BOOST_CHECK_EQUAL(rval, false);
}
BOOST_FIXTURE_TEST_CASE
	(router_findroute_connectedNodes_exceptSuccess, SimpleRouterFixture)
{
	d0->add_neighbour(d1);
	d1->add_neighbour(d0);

	bool rval = r.find_route
		(*d0, *d1,
		 &route,
		 wwWORKER,
		 -1,
		 cc);

	BOOST_CHECK_EQUAL(rval, true);
}

struct ComplexRouterFixture {
	typedef std::vector<RoutingNode *> Nodes;

	ComplexRouterFixture() : r(boost::bind(&ComplexRouterFixture::reset, this)) {
		d0 = new TestingRoutingNode();
		nodes.push_back(d0);
	}
	~ComplexRouterFixture() {
		while (!nodes.empty()) {
			RoutingNode * n = nodes.back();
			delete n;
			nodes.pop_back();
		}
	}

	/**
	  * Convenience function
	  */
	TestingRoutingNode * new_node_w_neighbour
		(TestingRoutingNode * const d,
		 Coords               const pos      = Coords(0, 0),
		 int32_t                             = 1,
		 int32_t              const waitcost = 0)
	{
		TestingRoutingNode * dnew = new TestingRoutingNode
			(waitcost, pos);

		dnew->add_neighbour(d);
		d->add_neighbour(dnew);

		nodes.push_back(dnew);
		return dnew;
	}

	/**
	  * Add a triangle of nodes (each node is connected to the
	  * other two) starting at the already existing node.
	  *
	  * \return The argument Node
	  */
	TestingRoutingNode * add_triangle(TestingRoutingNode * d) {
		TestingRoutingNode * dnew_1 = new TestingRoutingNode();
		TestingRoutingNode * dnew_2 = new TestingRoutingNode();

		d->add_neighbour(dnew_1); d->add_neighbour(dnew_2);
		dnew_1->add_neighbour(d); dnew_1->add_neighbour(dnew_2);
		dnew_2->add_neighbour(dnew_1); dnew_2->add_neighbour(d);

		nodes.push_back(dnew_1);
		nodes.push_back(dnew_2);
		return d;
	}

	/**
	  * Add a dead end to confuse the router
	  *
	  * \arg d The node to attach the dead end to
	  */
	TestingRoutingNode * add_dead_end(TestingRoutingNode * d) {

		// Some random dead ends
		TestingRoutingNode * d_new = new_node_w_neighbour(d);
		d_new = new_node_w_neighbour(d_new);

		TestingRoutingNode * dnew_2 = new_node_w_neighbour(d_new);
		new_node_w_neighbour(dnew_2);
		new_node_w_neighbour(dnew_2);
		dnew_2 = new_node_w_neighbour(dnew_2);
		new_node_w_neighbour(dnew_2);

		new_node_w_neighbour(d);
		new_node_w_neighbour(d_new);

		return d;
	}

	/**
	 * Add a chain of nodes
	 *
	 * \arg n number of entries in the chain
	 * \arg start First node in chain
	 * \arg chain All nodes will be appended to this chain. If n = 3 then
	 *      n->size() will be 4 at return
	 *
	 * \return The last node in the chain
	 */
	TestingRoutingNode * add_chain
		(int const n, TestingRoutingNode * const start, Nodes * const chain)
	{
		TestingRoutingNode * last = start;
		chain->push_back(start);
		for (int i = 0; i < n; i++) {
			last = new_node_w_neighbour(last);
			chain->push_back(last);
		}
		return last;
	}

	/**
	 * Callback for the incredibly rare case that the \ref Router pathfinding
	 * cycle wraps around.
	 */
	void  reset()
	{
		container_iterate(Nodes, nodes, i)
			(*i.current)->reset_path_finding_cycle();
	}
	TestingRoutingNode * d0;
	Nodes nodes;
	Router r;
	TestingRoute route;
	TestingTransportCostCalculator cc;
};

BOOST_FIXTURE_TEST_CASE(triangle_test, ComplexRouterFixture) {
	add_triangle(d0);

	BOOST_CHECK_EQUAL(nodes.size(), 3);
}

BOOST_FIXTURE_TEST_CASE(find_long_route, ComplexRouterFixture) {
	Nodes chain;

	TestingRoutingNode * d5 = add_chain(5, d0, &chain);

	new_node_w_neighbour(d0);

	bool rval = r.find_route
		(*d0, *d5,
		 &route,
		 wwWORKER,
		 -1,
		 cc);

	BOOST_CHECK_EQUAL(rval, true);

	add_dead_end(static_cast<TestingRoutingNode *>(chain[0]));
	add_dead_end(static_cast<TestingRoutingNode *>(chain[3]));
	add_dead_end(static_cast<TestingRoutingNode *>(chain[5]));

	BOOST_CHECK(route.has_chain(chain));

	// directly connect d0 -> d5
	d0->add_neighbour(d5);

	rval = r.find_route
		(*d0, *d5,
		 &route,
		 wwWORKER,
		 -1,
		 cc);

	BOOST_CHECK_EQUAL(rval, true);

	chain.clear();
	chain.push_back(d0);
	chain.push_back(d5);

	BOOST_CHECK(route.has_chain(chain));
}

/*************************************************************************/
/*                            Distance routing                           */
/*************************************************************************/
struct DistanceRoutingFixture : public ComplexRouterFixture {
	DistanceRoutingFixture() :
		ComplexRouterFixture() {
			// node is connected through a long and a short path
			// start d1 end
			start = d0;
			d1 = new_node_w_neighbour(start, Coords(1, 0));
			end = new_node_w_neighbour(d1, Coords(2, 0));

			// start d2 d3 d4 d5 end
			d2 = new_node_w_neighbour(start);
			d3 = new_node_w_neighbour(d2, Coords(0, 1));
			d4 = new_node_w_neighbour(d3, Coords(1, 2));
			d5 = new_node_w_neighbour(d4, Coords(1, 1));
			end->add_neighbour(d5);
			d5->add_neighbour(end);
		}

	TestingRoutingNode * start;
	TestingRoutingNode * end;
	TestingRoutingNode * d1, * d2, * d3, * d4, * d5;
};
BOOST_FIXTURE_TEST_CASE(priced_routing, DistanceRoutingFixture) {
	Nodes chain;
	chain.push_back(start);
	chain.push_back(d1);
	chain.push_back(end);

	bool rval = r.find_route
		(*start, *end,
		 &route,
		 wwWORKER,
		 -1,
		 cc);

	BOOST_CHECK(rval);
	BOOST_CHECK(route.has_chain(chain));

	// Make the middle node on the short path very expensive
	d1->set_waitcost(8);

	// Same result without wait
	rval = r.find_route
		(*start, *end,
		 &route,
		 wwWORKER,
		 -1,
		 cc);
	BOOST_CHECK(rval);
	BOOST_CHECK(route.has_chain(chain));

	// For wares, we now take the long route
	rval = r.find_route
		(*start, *end,
		 &route,
		 wwWARE,
		 -1,
		 cc);

	chain.clear();
	chain.push_back(start);
	chain.push_back(d2);
	chain.push_back(d3);
	chain.push_back(d4);
	chain.push_back(d5);
	chain.push_back(end);

	BOOST_CHECK(rval);

	BOOST_CHECK(route.has_chain(chain));
}
BOOST_FIXTURE_TEST_CASE(cutoff, DistanceRoutingFixture) {
	Nodes chain;

	RoutingNode * end_node = add_chain(4, d0, &chain);

	bool rval = r.find_route
		(*d0, *end_node,
		 &route,
		 wwWORKER,
		 1000,
		 cc);

	BOOST_CHECK_EQUAL(rval, false);
}


// }}}

BOOST_AUTO_TEST_SUITE_END()

/*
 * Copyright (C) 2007-2021 by the Widelands Development Team
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

#include "base/macros.h"
#include "base/test.h"
#include "economy/flag.h"
#include "economy/iroute.h"
#include "economy/itransport_cost_calculator.h"
#include "economy/router.h"
#include "economy/routing_node.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands_geometry.h"

/******************/
/* Helper classes */
/******************/
/// Helper classes {{{
class BadAccess : public std::exception {};
class TestingRoutingNode : public Widelands::RoutingNode {
public:
	explicit TestingRoutingNode(int32_t wcost = 0, Widelands::Coords pos = Widelands::Coords(0, 0))
	   : waitcost_(wcost), position_(pos) {
	}
	void add_neighbour(TestingRoutingNode* nb) {
		neighbours_.push_back(nb);
	}
	TestingRoutingNode* get_neighbour(uint8_t idx) const {
		if (idx >= neighbours_.size()) {
			throw BadAccess();
		}
		return neighbours_[idx];
	}

	Widelands::Flag& base_flag() override {
		return flag_;
	}
	void set_waitcost(int32_t const wc) {
		waitcost_ = wc;
	}
	int32_t get_waitcost() const {
		return waitcost_;
	}
	const Widelands::Coords& get_position() const override {
		return position_;
	}

	void get_neighbours(Widelands::WareWorker type, Widelands::RoutingNodeNeighbours&) override;

	// test functionality
	bool all_members_zeroed() const;

private:
	using Neigbours = std::vector<TestingRoutingNode*>;

	Neigbours neighbours_;
	int32_t waitcost_;
	Widelands::Coords position_;
	Widelands::Flag flag_;
};
void TestingRoutingNode::get_neighbours(Widelands::WareWorker type,
                                        Widelands::RoutingNodeNeighbours& n) {
	for (TestingRoutingNode* nb : neighbours_) {
		// second parameter is walktime in ms from this flag to the neighbour.
		// only depends on slope
		n.push_back(Widelands::RoutingNodeNeighbour(
		   nb, 1000 * ((type == Widelands::wwWARE) ? 1 + waitcost_ : 1)));
	}
}
bool TestingRoutingNode::all_members_zeroed() const {
	bool integers_zero = !mpf_cycle_ware && !mpf_realcost_ware && !mpf_estimate_ware &&
	                     !mpf_cycle_worker && !mpf_realcost_worker && !mpf_estimate_worker;
	bool pointers_zero = (mpf_backlink_ware == nullptr) && (mpf_backlink_worker == nullptr);

	return pointers_zero && integers_zero;
}

class TestingTransportCostCalculator : public Widelands::ITransportCostCalculator {
	int32_t calc_cost_estimate(const Widelands::Coords& c1,
	                           const Widelands::Coords& c2) const override {
		// We use an euclidian metric here. It is much easier for
		// test cases
		double xd = (c1.x - c2.x);
		double yd = (c1.y - c2.y);
		return static_cast<int32_t>((xd * xd + yd * yd) * 1000);
	}
};
class TestingRoute : public Widelands::IRoute {
public:
	using Nodes = std::vector<Widelands::RoutingNode*>;

	void init(int32_t) override {
		nodes.clear();
	}
	void insert_as_first(Widelands::RoutingNode* node) override {
		nodes.insert(nodes.begin(), node);
	}

	int32_t get_length() const {
		return nodes.size();
	}

	bool has_node(Widelands::RoutingNode* const n) {
		for (Widelands::RoutingNode* temp_node : nodes) {
			if (temp_node == n) {
				return true;
			}
		}
		return false;
	}
	bool has_chain(Nodes& n) {
		bool chain_begin_found = false;

		Nodes::iterator j = n.begin();
		Nodes::iterator i = nodes.begin();
		while (i != nodes.end() && j != n.end()) {
			if (!chain_begin_found) {
				if (*i == *j) {
					chain_begin_found = true;
					++j;
				}
				if (j == n.end()) {
					return true;
				}
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
TESTSUITE_START(Routing)

// {{{ TestingRoutingNode Test Cases
/*
 * First test the TestingRoutingNode class, so that we
 * are sure it works in the other tests
 *
 */
TESTCASE(testingnode_creation) {
	TestingRoutingNode d0;
	TestingRoutingNode d1(0, Widelands::Coords(15, 0));

	check_equal(d0.get_position().y, d1.get_position().y);
	check_equal(d0.get_position().x, 0);
	check_equal(d1.get_position().x, 15);
}
struct TestingNodeDefaultNodesFixture {
	TestingNodeDefaultNodesFixture() {
		d0 = new TestingRoutingNode();
		d1 = new TestingRoutingNode(1, Widelands::Coords(15, 0));
		nodes.push_back(d0);
		nodes.push_back(d1);
	}
	~TestingNodeDefaultNodesFixture() {
		while (!nodes.empty()) {
			TestingRoutingNode* n = nodes.back();
			delete n;
			nodes.pop_back();
		}
	}

	DISALLOW_COPY_AND_ASSIGN(TestingNodeDefaultNodesFixture);

	std::vector<TestingRoutingNode*> nodes;
	TestingRoutingNode* d0;
	TestingRoutingNode* d1;
};
TESTCASE(testingnode_neighbour_attaching) {
	TestingNodeDefaultNodesFixture f;
	f.d0->add_neighbour(f.d1);

	check_equal(f.d0->get_neighbour(0), f.d1);
}
TESTCASE(testingnode_illegalneighbour_access) {
	TestingNodeDefaultNodesFixture f;
	try {
		f.d0->get_neighbour(0);
	} catch (BadAccess&) {
		return;
	}
	throw wexception("BadAccess not thrown");
}
// }}} End of TestingRoutingNode Test cases

// {{{ RoutingNode Tests
/*
 * Now test the routing nodes functionality
 */
TESTCASE(RoutingNode_InitializeMemberVariables) {
	TestingRoutingNode d0(0, Widelands::Coords(15, 0));

	check_equal(true, d0.all_members_zeroed());
}

struct SimpleRouterFixture {
	SimpleRouterFixture() : r([this]() { reset(); }) {
		d0 = new TestingRoutingNode();
		d1 = new TestingRoutingNode(1, Widelands::Coords(15, 0));
		vec.push_back(d0);
		vec.push_back(d1);
	}
	~SimpleRouterFixture() {
		delete d0;
		delete d1;
	}

	DISALLOW_COPY_AND_ASSIGN(SimpleRouterFixture);

	/**
	 * Callback for the incredibly rare case that the \ref Router pathfinding
	 * cycle wraps around.
	 */
	void reset() {
		if (d0) {
			d0->reset_path_finding_cycle(Widelands::wwWARE);
			d0->reset_path_finding_cycle(Widelands::wwWORKER);
		}
		if (d1) {
			d1->reset_path_finding_cycle(Widelands::wwWARE);
			d1->reset_path_finding_cycle(Widelands::wwWORKER);
		}
	}
	TestingRoutingNode* d0;
	TestingRoutingNode* d1;
	std::vector<Widelands::RoutingNode*> vec;
	Widelands::Router r;
	TestingRoute route;
	TestingTransportCostCalculator cc;
};

// {{{ TestingRoute Testing
TESTCASE(TestingRoute_emptyatstart) {
	TestingRoute r;
	check_equal(r.get_length(), 0);
}
TESTCASE(TestingRoute_addnode) {
	TestingRoute r;
	TestingRoutingNode d;
	r.insert_as_first(&d);

	check_equal(r.get_length(), 1);
}
TESTCASE(TestingRoute_hasnode) {
	SimpleRouterFixture f;
	check_equal(f.route.has_node(f.d0), false);
	check_equal(f.route.has_node(f.d1), false);
	f.route.insert_as_first(f.d0);
	check_equal(f.route.has_node(f.d0), true);
	check_equal(f.route.has_node(f.d1), false);
	f.route.insert_as_first(f.d1);
	check_equal(f.route.has_node(f.d0), true);
	check_equal(f.route.has_node(f.d1), true);
}
TESTCASE(TestingRoute_haschain) {
	SimpleRouterFixture f;
	std::vector<Widelands::RoutingNode*> chain;

	chain.push_back(f.d0);
	f.route.insert_as_first(f.d0);
	check_equal(f.route.has_chain(chain), true);

	f.route.insert_as_first(f.d1);
	chain.insert(chain.begin(), f.d1);
	check_equal(f.route.has_chain(chain), true);
}
TESTCASE(TestingRoute_chainisunidirectional) {
	SimpleRouterFixture f;
	std::vector<Widelands::RoutingNode*> chain;
	// Chains are unidirectional. Check that chain.clear();
	chain.push_back(f.d0);
	chain.push_back(f.d1);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(f.d1);
	check_equal(f.route.has_chain(chain), false);
}
TESTCASE(TestingRoute_haschain_checksubchain) {
	SimpleRouterFixture f;
	// Do not get confused when a partial chain is found
	TestingRoutingNode d;
	std::vector<Widelands::RoutingNode*> chain;

	chain.push_back(&d);
	chain.push_back(f.d0);
	chain.push_back(f.d1);

	f.route.insert_as_first(&d);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(&d);  // d -> d0 -> d
	check_equal(f.route.has_chain(chain), false);
	f.route.insert_as_first(f.d1);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(&d);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(f.d1);  // d1 d0 d d0 d1 d d0 d
	check_equal(f.route.has_chain(chain), true);
}
TESTCASE(TestingRoute_haschain_checksubchain_endisnotstart) {
	SimpleRouterFixture f;
	// Do not get confused when a partial chain is found
	TestingRoutingNode d;
	std::vector<Widelands::RoutingNode*> chain;

	chain.push_back(&d);
	chain.push_back(f.d0);
	chain.push_back(f.d1);

	f.route.insert_as_first(&d);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(f.d0);
	check_equal(f.route.has_chain(chain), false);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(&d);
	f.route.insert_as_first(f.d0);
	check_equal(f.route.has_chain(chain), false);
	f.route.insert_as_first(f.d1);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(&d);
	f.route.insert_as_first(f.d0);
	f.route.insert_as_first(f.d1);
	check_equal(f.route.has_chain(chain), true);
}
TESTCASE(TestingRoute_init) {
	SimpleRouterFixture f;
	f.route.insert_as_first(f.d0);
	check_equal(f.route.get_length(), 1);
	f.route.init(0);
	check_equal(f.route.get_length(), 0);
	f.route.insert_as_first(f.d1);
	check_equal(f.route.get_length(), 1);
}
// }}}

// {{{ Router Test-Cases
/*************************************************************************/
/*                           EQUAL COST TESTS                            */
/*************************************************************************/
/****************/
/* SIMPLE TESTS */
/****************/
TESTCASE(router_findroute_seperatedNodes_exceptFail) {
	SimpleRouterFixture f;
	bool rval = f.r.find_route(*f.d0, *f.d1, &f.route, Widelands::wwWORKER, -1, f.cc);

	check_equal(rval, false);
}
TESTCASE(router_findroute_connectedNodes_exceptSuccess) {
	SimpleRouterFixture f;
	f.d0->add_neighbour(f.d1);
	f.d1->add_neighbour(f.d0);

	bool rval = f.r.find_route(*f.d0, *f.d1, &f.route, Widelands::wwWORKER, -1, f.cc);

	check_equal(rval, true);
}

struct ComplexRouterFixture {
	using Nodes = std::vector<Widelands::RoutingNode*>;

	ComplexRouterFixture() : r([this]() { reset(); }) {
		d0 = new TestingRoutingNode();
		nodes.push_back(d0);
	}
	~ComplexRouterFixture() {
		while (!nodes.empty()) {
			Widelands::RoutingNode* n = nodes.back();
			delete n;
			nodes.pop_back();
		}
	}

	DISALLOW_COPY_AND_ASSIGN(ComplexRouterFixture);

	/**
	 * Convenience function
	 */
	TestingRoutingNode* new_node_w_neighbour(TestingRoutingNode* const d,
	                                         const Widelands::Coords& pos = Widelands::Coords(0, 0),
	                                         int32_t = 1,
	                                         int32_t const waitcost = 0) {
		TestingRoutingNode* dnew = new TestingRoutingNode(waitcost, pos);

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
	TestingRoutingNode* add_triangle(TestingRoutingNode* d) {
		TestingRoutingNode* dnew_1 = new TestingRoutingNode();
		TestingRoutingNode* dnew_2 = new TestingRoutingNode();

		d->add_neighbour(dnew_1);
		d->add_neighbour(dnew_2);
		dnew_1->add_neighbour(d);
		dnew_1->add_neighbour(dnew_2);
		dnew_2->add_neighbour(dnew_1);
		dnew_2->add_neighbour(d);

		nodes.push_back(dnew_1);
		nodes.push_back(dnew_2);
		return d;
	}

	/**
	 * Add a dead end to confuse the router
	 *
	 * \arg d The node to attach the dead end to
	 */
	TestingRoutingNode* add_dead_end(TestingRoutingNode* d) {

		// Some random dead ends
		TestingRoutingNode* d_new = new_node_w_neighbour(d);
		d_new = new_node_w_neighbour(d_new);

		TestingRoutingNode* dnew_2 = new_node_w_neighbour(d_new);
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
	TestingRoutingNode* add_chain(int const n, TestingRoutingNode* const start, Nodes* const chain) {
		TestingRoutingNode* last = start;
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
	void reset() {
		for (Widelands::RoutingNode* node : nodes) {
			node->reset_path_finding_cycle(Widelands::wwWARE);
			node->reset_path_finding_cycle(Widelands::wwWORKER);
		}
	}
	TestingRoutingNode* d0;
	Nodes nodes;
	Widelands::Router r;
	TestingRoute route;
	TestingTransportCostCalculator cc;
};

TESTCASE(triangle_test) {
	ComplexRouterFixture f;
	f.add_triangle(f.d0);

	check_equal(f.nodes.size(), 3);
}

TESTCASE(find_long_route) {
	ComplexRouterFixture f;
	ComplexRouterFixture::Nodes chain;

	TestingRoutingNode* d5 = f.add_chain(5, f.d0, &chain);

	f.new_node_w_neighbour(f.d0);

	bool rval = f.r.find_route(*f.d0, *d5, &f.route, Widelands::wwWORKER, -1, f.cc);

	check_equal(rval, true);

	f.add_dead_end(dynamic_cast<TestingRoutingNode*>(chain[0]));
	f.add_dead_end(dynamic_cast<TestingRoutingNode*>(chain[3]));
	f.add_dead_end(dynamic_cast<TestingRoutingNode*>(chain[5]));

	check_equal(true, f.route.has_chain(chain));

	// directly connect d0 -> d5
	f.d0->add_neighbour(d5);

	rval = f.r.find_route(*f.d0, *d5, &f.route, Widelands::wwWORKER, -1, f.cc);

	check_equal(rval, true);

	chain.clear();
	chain.push_back(f.d0);
	chain.push_back(d5);

	check_equal(true, f.route.has_chain(chain));
}

/*************************************************************************/
/*                            Distance routing                           */
/*************************************************************************/
struct DistanceRoutingFixture : public ComplexRouterFixture {
	DistanceRoutingFixture() {
		// node is connected through a long and a short path
		// start d1 end
		start = d0;
		d1 = new_node_w_neighbour(start, Widelands::Coords(1, 0));
		end = new_node_w_neighbour(d1, Widelands::Coords(2, 0));

		// start d2 d3 d4 d5 end
		d2 = new_node_w_neighbour(start);
		d3 = new_node_w_neighbour(d2, Widelands::Coords(0, 1));
		d4 = new_node_w_neighbour(d3, Widelands::Coords(1, 2));
		d5 = new_node_w_neighbour(d4, Widelands::Coords(1, 1));
		end->add_neighbour(d5);
		d5->add_neighbour(end);
	}

	TestingRoutingNode* start;
	TestingRoutingNode* end;
	TestingRoutingNode *d1, *d2, *d3, *d4, *d5;
};
TESTCASE(priced_routing) {
	DistanceRoutingFixture f;
	DistanceRoutingFixture::Nodes chain;
	chain.push_back(f.start);
	chain.push_back(f.d1);
	chain.push_back(f.end);

	bool rval = f.r.find_route(*f.start, *f.end, &f.route, Widelands::wwWORKER, -1, f.cc);

	check_equal(true, rval);
	check_equal(true, f.route.has_chain(chain));

	// Make the middle node on the short path very expensive
	f.d1->set_waitcost(8);

	// Same result without wait
	rval = f.r.find_route(*f.start, *f.end, &f.route, Widelands::wwWORKER, -1, f.cc);
	check_equal(true, rval);
	check_equal(true, f.route.has_chain(chain));

	// For wares, we now take the long route
	rval = f.r.find_route(*f.start, *f.end, &f.route, Widelands::wwWARE, -1, f.cc);

	chain.clear();
	chain.push_back(f.start);
	chain.push_back(f.d2);
	chain.push_back(f.d3);
	chain.push_back(f.d4);
	chain.push_back(f.d5);
	chain.push_back(f.end);

	check_equal(true, rval);

	check_equal(true, f.route.has_chain(chain));
}
TESTCASE(cutoff) {
	DistanceRoutingFixture f;
	DistanceRoutingFixture::Nodes chain;

	Widelands::RoutingNode* end_node = f.add_chain(4, f.d0, &chain);

	bool rval = f.r.find_route(*f.d0, *end_node, &f.route, Widelands::wwWORKER, 1000, f.cc);

	check_equal(rval, false);
}

// }}}

TESTSUITE_END()

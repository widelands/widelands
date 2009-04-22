/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_geometry.h"

#include "../iroute.h"
#include "../itransport_cost_calculator.h"
#include "../router.h"
#include "../routing_node.h"


#include <exception>

#include <boost/test/unit_test.hpp>

using namespace Widelands;

/******************/
/* Helper classes */
/******************/
/// Helper classes {{{
class BadAccess : public std::exception {};
class DummyRoutingNode : public RoutingNode {
public:
    DummyRoutingNode( int32_t rcost = 1, int32_t wcost = 0, Coords pos = Coords(0,0) ) :
         _waitcost(wcost), _position(pos) {
    }
    void add_neighbour( DummyRoutingNode* nb ) {
        _neighbours.push_back(nb);
    }
    DummyRoutingNode* get_neighbour( uint8_t idx ) {
        if( idx >= _neighbours.size() )
            throw BadAccess();
        return _neighbours[idx];
    }

    void set_waitcost(int32_t wc) { _waitcost = wc; }
    int32_t get_waitcost(void) { return _waitcost; }
    Coords get_position() const { return _position; } 
	
    void get_neighbours(RoutingNodeNeighbours *);

private:
    typedef std::vector<DummyRoutingNode*> Neigbours;

    Neigbours _neighbours;
    int32_t _waitcost;
    Coords _position;
};
void DummyRoutingNode::get_neighbours(RoutingNodeNeighbours* n ) {
    for (Neigbours::iterator i = _neighbours.begin(); i != _neighbours.end();
            i++) {
        RoutingNodeNeighbour nb(*i, 1000); // second parameter is walktime in ms from this flag to the neighbour. only depends on slope
        n->push_back(nb);
    }
}

class DummyTransportCostCalculator : public ITransportCostCalculator {
	int32_t calc_cost_estimate(Coords c1, Coords c2) const { 
        // We use an euclidian metric here. It is much easier for
        // test cases
        double xd = (c1.x -c2.x);
        double yd = (c2.y- c2.y);
        return static_cast<int32_t>((xd*xd+yd*yd)*1000);
    } 
};
class DummyRoute : public IRoute {
public:
    typedef std::vector<RoutingNode*> Nodes;

	void init( int32_t ) { 
        nodes.clear();
    } 
    void insert_as_first(RoutingNode* node) { 
        nodes.insert(nodes.begin(), node);
    } 

    int32_t get_length(void) { return nodes.size(); }

    bool has_node(RoutingNode* n) {
        for( Nodes::iterator i = nodes.begin(); i != nodes.end(); i++) {
            if(*i == n)
                return true;
        }
        return false; 
    }
    bool has_chain(Nodes& n) {
        bool chain_begin_found = false;
        
        Nodes::iterator j = n.begin();
        Nodes::iterator i = nodes.begin();
        while( i < nodes.end() && j < n.end()) {
            if(!chain_begin_found) {
                if(*i == *j) {
                    chain_begin_found = true;
                    ++j;
                }
                if(j == n.end()) 
                        return true;
            } else {
                if(*i != *j) {
                    j = n.begin();
                    chain_begin_found=false;
                    if(*i == *j) {
                        chain_begin_found = true;
                        j++;
                    }
                } else {
                    j++;
                    if(j == n.end()) {
                        return true;
                    }
                }
            }
            ++i;
        }

        return false; 
    }

    bool print(void) {
        for(Nodes::iterator i = nodes.begin(); i < nodes.end(); i++) {
            BOOST_MESSAGE( *i );
        }
    }
private:
    Nodes nodes;
};

/// End of helper classes }}}


/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
BOOST_AUTO_TEST_SUITE( Routing )

/// {{{ DummyRoutingNode Test Cases
/* 
 * First test the DummyRoutingNode class, so that we 
 * are sure it works in the other tests
 *
 */
BOOST_AUTO_TEST_CASE( dummynode_creation ) {
    DummyRoutingNode d0;
    DummyRoutingNode d1(0,0,Coords(15,0));

    BOOST_CHECK_EQUAL(d0.get_position().y,d1.get_position().y);
    BOOST_CHECK_EQUAL(d0.get_position().x,0);
    BOOST_CHECK_EQUAL(d1.get_position().x,15);
}
struct DummyNode_DefaultNodes_Fixture {
    DummyNode_DefaultNodes_Fixture( void ) {
        d0 = new DummyRoutingNode();
        d1 = new DummyRoutingNode(1,Coords(15,0));
        nodes.push_back(d0);
        nodes.push_back(d1);
    }
    ~DummyNode_DefaultNodes_Fixture( void ) {
        while(nodes.size()) {
            DummyRoutingNode* n = nodes.back();
            delete n;
            nodes.pop_back();
        }
    }
    std::vector<DummyRoutingNode*> nodes;
    DummyRoutingNode* d0;
    DummyRoutingNode* d1;
};
BOOST_FIXTURE_TEST_CASE( dummynode_neighbour_attaching, DummyNode_DefaultNodes_Fixture) {
    d0->add_neighbour(d1);
    
    BOOST_CHECK_EQUAL( d0->get_neighbour(0), d1 );
}
BOOST_FIXTURE_TEST_CASE( dummynode_illegalneighbour_access, DummyNode_DefaultNodes_Fixture) {
    try {
        d0->get_neighbour(0);
        BOOST_ERROR("BadAccess not thrown"); 
    } catch( BadAccess& ) {
    }
}
// }}} End of DummyRoutingNode Test cases

struct SimpleRouterFixture {
    SimpleRouterFixture( void ) {
        d0 = new DummyRoutingNode();
        d1 = new DummyRoutingNode(1,Coords(15,0));
        vec.push_back(d0);
        vec.push_back(d1);
    }
    ~SimpleRouterFixture( ) {
        delete d0; 
        delete d1;
    }
    DummyRoutingNode* d0;
    DummyRoutingNode* d1;
    std::vector<RoutingNode*> vec;
    Router r; 
    DummyRoute route;
    DummyTransportCostCalculator cc;
};

// {{{ DummyRoute Testing
BOOST_AUTO_TEST_CASE( DummyRoute_emptyatstart ) {
    DummyRoute r;
    BOOST_CHECK_EQUAL( r.get_length(), 0 );
}
BOOST_AUTO_TEST_CASE( DummyRoute_addnode ) {
    DummyRoute r;
    DummyRoutingNode d;
    r.insert_as_first( &d );
    
    BOOST_CHECK_EQUAL( r.get_length(), 1 );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_hasnode, SimpleRouterFixture ) {
    BOOST_CHECK_EQUAL( route.has_node(d0), false );
    BOOST_CHECK_EQUAL( route.has_node(d1), false );
    route.insert_as_first( d0 );
    BOOST_CHECK_EQUAL( route.has_node(d0), true );
    BOOST_CHECK_EQUAL( route.has_node(d1), false );
    route.insert_as_first( d1 );
    BOOST_CHECK_EQUAL( route.has_node(d0), true );
    BOOST_CHECK_EQUAL( route.has_node(d1), true );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_haschain, SimpleRouterFixture ) {
    std::vector<RoutingNode*> chain;

    chain.push_back(d0);
    route.insert_as_first(d0);
    BOOST_CHECK_EQUAL( route.has_chain(chain), true );

    route.insert_as_first(d1);
    chain.insert(chain.begin(), d1);
    BOOST_CHECK_EQUAL( route.has_chain(chain), true );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_chainisunidirectional, SimpleRouterFixture ) {
    std::vector<RoutingNode*> chain;
    // Chains are unidirectional. Check that
    chain.clear();
    chain.push_back(d0);
    chain.push_back(d1);
    route.insert_as_first(d0);
    route.insert_as_first(d1);
    BOOST_CHECK_EQUAL( route.has_chain(chain), false );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_haschain_checksubchain, SimpleRouterFixture ) {
    // Do not get confused when a partial chain is found
    DummyRoutingNode d;
    std::vector<RoutingNode*> chain;
    
    chain.push_back(&d);
    chain.push_back(d0);
    chain.push_back(d1);

    route.insert_as_first(&d);
    route.insert_as_first(d0);
    route.insert_as_first(&d); // d -> d0 -> d
    BOOST_CHECK_EQUAL( route.has_chain(chain), false );
    route.insert_as_first(d1);
    route.insert_as_first(d0);
    route.insert_as_first(&d);
    route.insert_as_first(d0);
    route.insert_as_first(d1); // d1 d0 d d0 d1 d d0 d
    BOOST_CHECK_EQUAL( route.has_chain(chain), true );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_haschain_checksubchain_endisnotstart, SimpleRouterFixture ) {
    // Do not get confused when a partial chain is found
    DummyRoutingNode d;
    std::vector<RoutingNode*> chain;
    
    chain.push_back(&d);
    chain.push_back(d0);
    chain.push_back(d1);

    route.insert_as_first(&d);
    route.insert_as_first(d0);
    route.insert_as_first(d0);
    BOOST_CHECK_EQUAL( route.has_chain(chain), false );
    route.insert_as_first(d0);
    route.insert_as_first(d0);
    route.insert_as_first(&d);
    route.insert_as_first(d0);
    BOOST_CHECK_EQUAL( route.has_chain(chain), false );
    route.insert_as_first(d1);
    route.insert_as_first(d0);
    route.insert_as_first(&d);
    route.insert_as_first(d0);
    route.insert_as_first(d1);
    BOOST_CHECK_EQUAL( route.has_chain(chain), true );
}
BOOST_FIXTURE_TEST_CASE( DummyRoute_init, SimpleRouterFixture ) {
    route.insert_as_first( d0 );
    BOOST_CHECK_EQUAL( route.get_length(), 1 );
    route.init(0);
    BOOST_CHECK_EQUAL( route.get_length(), 0 );
    route.insert_as_first( d1 );
    BOOST_CHECK_EQUAL( route.get_length(), 1 );
}
// }}}

// {{{ Router Test-Cases 
/*************************************************************************/
/*                           EQUAL COST TESTS                            */
/*************************************************************************/
/****************/
/* SIMPLE TESTS */
/****************/
BOOST_FIXTURE_TEST_CASE( router_findroute_seperatedNodes_exceptFail, SimpleRouterFixture) {
    bool rval = r.find_route( *d0, *d1, 
        &route, 
        false, 
        -1,
        cc,
        vec);

    BOOST_CHECK_EQUAL(rval,false);
}
BOOST_FIXTURE_TEST_CASE( router_findroute_connectedNodes_exceptSuccess, SimpleRouterFixture) {
    d0->add_neighbour(d1);
    d1->add_neighbour(d0);
    bool rval = r.find_route( *d0, *d1, 
        &route, 
        false, 
        -1,
        cc,
        vec);

    BOOST_CHECK_EQUAL(rval,true);
}

struct ComplexRouterFixture {
    typedef std::vector<RoutingNode*> Nodes;

    ComplexRouterFixture() {
        d0 = new DummyRoutingNode();
        nodes.push_back(d0);
    }
    ~ComplexRouterFixture() { 
        while(nodes.size()) {
            RoutingNode* n = nodes.back();
            delete n;
            nodes.pop_back();
        }
    }
  
    /**
     * Convenience function
     */
    DummyRoutingNode* new_node_w_neighbour( DummyRoutingNode* d, 
			Coords pos = Coords(0,0), 
			int32_t roadcost = 1, int32_t waitcost = 0 ) {
        DummyRoutingNode* d0 = new DummyRoutingNode(roadcost, waitcost, pos);

        d0->add_neighbour(d);
        d->add_neighbour(d0);

        nodes.push_back(d0);
        return d0;
    }

    /**
     * Add a triangle of nodes (each node is connected to the
     * other two) starting at the already existing node.
     *
     * \return The argument Node
     */
    DummyRoutingNode* add_triangle( DummyRoutingNode* d ) {
        DummyRoutingNode* d0 = new DummyRoutingNode();
        DummyRoutingNode* d1 = new DummyRoutingNode();

        d->add_neighbour(d0); d->add_neighbour(d1);
        d0->add_neighbour(d); d0->add_neighbour(d1);
        d1->add_neighbour(d0); d1->add_neighbour(d);
        
        nodes.push_back(d0);
        nodes.push_back(d1);
        return d;
    }

    /**
     * Add a dead end to confuse the router
     *
     * \arg d The node to attach the dead end to
     */
    DummyRoutingNode* add_dead_end( DummyRoutingNode* d ) {

        // Some random dead ends
        DummyRoutingNode* d0 = new_node_w_neighbour(d);
        d0 = new_node_w_neighbour(d0);
        
        DummyRoutingNode* d1 = new_node_w_neighbour(d0);
        new_node_w_neighbour(d1);
        new_node_w_neighbour(d1);
        d1 = new_node_w_neighbour(d1);
        d1 = new_node_w_neighbour(d1);
    
        new_node_w_neighbour(d);
        new_node_w_neighbour(d0);

        return d;
    }

    DummyRoutingNode* d0;
    Nodes nodes;
    Router r; 
    DummyRoute route;
    DummyTransportCostCalculator cc;
};

BOOST_FIXTURE_TEST_CASE( triangle_test, ComplexRouterFixture ) {
    add_triangle(d0);

    BOOST_CHECK_EQUAL( nodes.size(), 3 );
}

BOOST_FIXTURE_TEST_CASE( find_long_route, ComplexRouterFixture ) {
    DummyRoutingNode* d1 = new_node_w_neighbour(d0);
    DummyRoutingNode* d2 = new_node_w_neighbour(d1);
    DummyRoutingNode* d3 = new_node_w_neighbour(d2);
    DummyRoutingNode* d4 = new_node_w_neighbour(d3);
    DummyRoutingNode* d5 = new_node_w_neighbour(d4);
    
    bool rval = r.find_route( *d0, *d5, 
        &route, 
        false, 
        -1,
        cc,
        nodes);
    
    BOOST_CHECK_EQUAL( rval, true );

    Nodes chain;
    chain.push_back(d0);
    chain.push_back(d1);
    chain.push_back(d2);
    chain.push_back(d3);
    chain.push_back(d4);
    chain.push_back(d5);

    add_dead_end(d0);
    add_dead_end(d3);
    add_dead_end(d5);

    BOOST_CHECK( route.has_chain(chain));
    
    // directly connect d0 -> d5
    d0->add_neighbour(d5);
    
    rval = r.find_route( *d0, *d5, 
        &route, 
        false, 
        -1,
        cc,
        nodes);
    
    BOOST_CHECK_EQUAL( rval, true );

    chain.clear();
    chain.push_back(d0);
    chain.push_back(d5);
    
    BOOST_CHECK( route.has_chain(chain));
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
        d1 = new_node_w_neighbour(start, Coords(1,0));
        end = new_node_w_neighbour(d1, Coords(2,0));
       
        // start d2 d3 d4 d5 end
        d2 = new_node_w_neighbour(start);
        d3 = new_node_w_neighbour(d2,Coords(0,1));
        d4 = new_node_w_neighbour(d3,Coords(1,2));
        d5 = new_node_w_neighbour(d4,Coords(1,1));
        end->add_neighbour(d5);
        d5->add_neighbour(end);
    }

    DummyRoutingNode* start; 
    DummyRoutingNode* end;
    DummyRoutingNode *d1, *d2, *d3, *d4, *d5;
};
BOOST_FIXTURE_TEST_CASE( priced_routing, DistanceRoutingFixture) {
    Nodes chain;
    chain.push_back(start);
    chain.push_back(d1);
    chain.push_back(end);
    
    bool rval = r.find_route( *start, *end,
        &route,
        false,
        -1,
        cc,
        nodes);

    BOOST_CHECK( rval );
    BOOST_CHECK( route.has_chain(chain) );

    // Make the middle node on the short path very expensive
    d1->set_waitcost( 6 );

    // Same result without wait
    rval = r.find_route( *start, *end,
        &route,
        false,
        -1,
        cc,
        nodes);
    BOOST_CHECK( rval );
    BOOST_CHECK( route.has_chain(chain) );

    // With wait, we take the long route
    rval = r.find_route( *start, *end,
        &route,
        true,
        -1,
        cc,
        nodes);

    // Same result without wait
    chain.clear(); 
    chain.push_back(start);
    chain.push_back(d2);
    chain.push_back(d3);
    chain.push_back(d4);
    chain.push_back(d5);
    chain.push_back(end);

    BOOST_CHECK( rval );
    route.print();

    BOOST_CHECK( route.has_chain(chain) );

}

// }}}

BOOST_AUTO_TEST_SUITE_END()


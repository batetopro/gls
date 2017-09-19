/******************************************************************************
 * graph_access.h
 *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 *
 ******************************************************************************
 * Copyright (C) 2013-2015 Christian Schulz <christian.schulz@kit.edu>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

typedef uint32_t NodeID;
typedef uint32_t EdgeID;
typedef uint32_t Color;

struct Node {
    EdgeID firstEdge;
};

struct Edge {
    NodeID target;
};

class graph_access;

//construction etc. is encapsulated in basicGraph / access to properties etc. is encapsulated in graph_access
class basicGraph {
    friend class graph_access;

public:
    basicGraph() : m_building_graph(false) {
    }

private:
    //methods only to be used by friend class
    EdgeID number_of_edges() {
        return m_edges.size();
    }

    NodeID number_of_nodes() {
        return m_nodes.size() - 1;
    }

    inline EdgeID get_first_edge(const NodeID & node) {
        return m_nodes[node];
    }

    inline EdgeID get_first_invalid_edge(const NodeID & node) {
        return m_nodes[node + 1];
    }

    // construction of the graph
    void start_construction(NodeID n, EdgeID m) {
        m_building_graph = true;
        node             = 0;
        e                = 0;
        m_last_source    = -1;

        //resizes property arrays
        m_nodes.resize(n + 1);
        m_edges.resize(m);

        m_nodes[node] = e;
    }

    EdgeID new_edge(NodeID source, NodeID target) {
        assert(m_building_graph);
        assert(e < m_edges.size());

        m_edges[e] = target;
        EdgeID e_bar = e;
        ++e;

        assert(source + 1 < m_nodes.size());
        m_nodes[source + 1] = e;

        //fill isolated sources at the end
        if ((NodeID)(m_last_source + 1) < source) {
            for (NodeID i = source; i > (NodeID)(m_last_source + 1); i--) {
                m_nodes[i] = m_nodes[m_last_source + 1];
            }
        }
        m_last_source = source;
        return e_bar;
    }

    NodeID new_node() {
        assert(m_building_graph);
        return node++;
    }

    void finish_construction() {
        // inert dummy node
        m_nodes.resize(node + 1);
        m_edges.resize(e);

        m_building_graph = false;

        //fill isolated sources at the end
        if ((unsigned int)(m_last_source) != node - 1) {
            //in that case at least the last node was an isolated node
            for (NodeID i = node; i > (unsigned int)(m_last_source + 1); i--) {
                m_nodes[i] = m_nodes[m_last_source + 1];
            }
        }
    }

    // %%%%%%%%%%%%%%%%%%% DATA %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // split properties for coarsening and uncoarsening
    std::vector<EdgeID> m_nodes;
    std::vector<NodeID> m_edges;

    // construction properties
    bool m_building_graph;
    int m_last_source;
    NodeID node; //current node that is constructed
    EdgeID e;    //current edge that is constructed
};

// //makros - graph access
// #define forall_edges(G,e) { for(EdgeID e = 0, end = G.number_of_edges(); e < end; ++e) {
// #define forall_nodes(G,n) { for(NodeID n = 0, end = G.number_of_nodes(); n < end; ++n) {
// #define forall_out_edges(G,e,n) { for(EdgeID e = G.get_first_edge(n), end = G.get_first_invalid_edge(n); e < end; ++e) {
// #define endfor }}

class graph_access {
public:
    graph_access() { m_max_degree_computed = false; m_max_degree = 0; graphref = new basicGraph();}
    virtual ~graph_access() { delete graphref; };

    /* ============================================================= */
    /* build methods */
    /* ============================================================= */
    void start_construction(NodeID nodes, EdgeID edges);
    NodeID new_node();
    EdgeID new_edge(NodeID source, NodeID target);
    void finish_construction();

    /* ============================================================= */
    /* graph access methods */
    /* ============================================================= */
    NodeID number_of_nodes() const;
    EdgeID number_of_edges() const;

    EdgeID get_first_edge(NodeID node) const;
    EdgeID get_first_invalid_edge(NodeID node) const;

    EdgeID getNodeDegree(NodeID node) const;
    EdgeID getMaxDegree();

    NodeID getEdgeTarget(EdgeID edge) const;

    class adjacency_iterator {
    public:
        adjacency_iterator(const graph_access& _G, EdgeID _e)
            :   G(_G)
            ,   e(_e)
        {}

        NodeID operator* () const {
            return G.getEdgeTarget(e);
        }

        adjacency_iterator operator++ () {
            ++e;
            return *this;
        }

        bool operator== (const adjacency_iterator& other) const {
            return e == other.e;
        }

        bool operator!= (const adjacency_iterator& other) const {
            return !(*this == other);
        }

    private:
        const graph_access& G;
        EdgeID e;
    };

    class adjacency_adapter {
    public:
        adjacency_adapter(const graph_access& _G, NodeID _n)
            :   G(_G)
            ,   n(_n)
        {}

        adjacency_iterator begin() {
            return adjacency_iterator(G, G.get_first_edge(n));
        }

        adjacency_iterator end() {
            return adjacency_iterator(G, G.get_first_invalid_edge(n));
        }

    private:
        const graph_access& G;
        NodeID n;
    };

    adjacency_adapter neighbours(NodeID n) const {
            return adjacency_adapter(*this, n);
    }
private:
    basicGraph* graphref;
    bool m_max_degree_computed;
    EdgeID m_max_degree;

};

/* graph build methods */
inline void graph_access::start_construction(NodeID nodes, EdgeID edges) {
    graphref->start_construction(nodes, edges);
}

inline NodeID graph_access::new_node() {
    return graphref->new_node();
}

inline EdgeID graph_access::new_edge(NodeID source, NodeID target) {
    return graphref->new_edge(source, target);
}

inline void graph_access::finish_construction() {
    graphref->finish_construction();
}

/* graph access methods */
inline NodeID graph_access::number_of_nodes() const {
    return graphref->number_of_nodes();
}

inline EdgeID graph_access::number_of_edges() const {
    return graphref->number_of_edges();
}

inline EdgeID graph_access::get_first_edge(NodeID node) const {
#ifdef NDEBUG
    return graphref->m_nodes[node];
#else
    return graphref->m_nodes.at(node);
#endif
}

inline EdgeID graph_access::get_first_invalid_edge(NodeID node) const {
    return graphref->m_nodes[node + 1];
}

inline NodeID graph_access::getEdgeTarget(EdgeID edge) const {
#ifdef NDEBUG
    return graphref->m_edges[edge];
#else
    return graphref->m_edges.at(edge);
#endif
}

inline EdgeID graph_access::getNodeDegree(NodeID node) const {
    return graphref->m_nodes[node + 1] - graphref->m_nodes[node];
}

inline EdgeID graph_access::getMaxDegree() {
    if (!m_max_degree_computed) {
        for (NodeID node = 0; node < number_of_nodes(); ++node) {
            EdgeID cur_degree = 0;
            for (auto e : neighbours(node)) {
                ++cur_degree;
            }
            if (cur_degree > m_max_degree) {
                m_max_degree = cur_degree;
            }
        }
        m_max_degree_computed = true;
    }

    return m_max_degree;
}

// for(EdgeID e = 0; e < G.number_of_edges(); ++e) { ... }
// for(NodeID n = 0; n < G.number_of_nodes(); ++n) { ... }

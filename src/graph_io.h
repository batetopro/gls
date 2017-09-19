/******************************************************************************
 * graph_io.h
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

#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "graph.h"

class graph_io {
public:
    graph_io();
    virtual ~graph_io () ;

    static int readGraphWeighted(graph_access& G, std::string filename);
};

int graph_io::readGraphWeighted(graph_access & G, std::string filename) {
    std::string line;

    // open file for reading
    std::ifstream in(filename.c_str());
    if (!in) {
        std::cerr << "Error opening " << filename << std::endl;
        return 1;
    }

    long nmbNodes;
    long nmbEdges;

    std::getline(in, line);
    //skip comments
    while ( line[0] == '%' ) {
        std::getline(in, line);
    }

    int ew = 0;
    std::stringstream ss(line);
    ss >> nmbNodes;
    ss >> nmbEdges;
    ss >> ew;

    if ( 2 * nmbEdges > std::numeric_limits<int>::max() || nmbNodes > std::numeric_limits<int>::max()) {
        std::cerr <<  "The graph is too large. Currently only 32bit supported!"  << std::endl;
        exit(0);
    }

    bool read_ew = false;
    bool read_nw = false;

    if (ew == 1) {
        read_ew = true;
    } else if (ew == 11) {
        read_ew = true;
        read_nw = true;
    } else if (ew == 10) {
        read_nw = true;
    }
    nmbEdges *= 2; //since we have forward and backward edges

    NodeID node_counter   = 0;
    EdgeID edge_counter   = 0;
    long long total_nodeweight = 0;

    G.start_construction(nmbNodes, nmbEdges);

    while (  std::getline(in, line)) {

        if (line[0] == '%') { // a comment in the file
            continue;
        }

        NodeID node = G.new_node();
        node_counter++;
        std::stringstream ss(line);

        NodeID weight = 1;
        if ( read_nw ) {
            ss >> weight;
            total_nodeweight += weight;
            if ( total_nodeweight > (long long) std::numeric_limits<NodeID>::max()) {
                std::cerr <<  "The sum of the node weights is too large (it exceeds the node weight type)."  << std::endl;
                std::cerr <<  "Currently not supported. Please scale your node weights."  << std::endl;
                exit(0);
            }
        }

        NodeID target;
        while ( ss >> target ) {
            //check for self-loops
            if (target - 1 == node) {
                std::cerr << "The graph file contains self-loops. This is not supported. Please remove them from the file."  << std::endl;
			}

            EdgeID edge_weight = 1;
            if ( read_ew ) {
                ss >> edge_weight;
            }
            edge_counter++;
            EdgeID e = G.new_edge(node, target - 1);
        }

        if (in.eof()) {
            break;
        }
    }

    if ( edge_counter != (EdgeID) nmbEdges ) {
        std::cerr <<  "number of specified edges mismatch"  << std::endl;
        std::cerr <<  edge_counter <<  " " <<  nmbEdges  << std::endl;
        exit(0);
    }

    if ( node_counter != (NodeID) nmbNodes) {
        std::cerr <<  "number of specified nodes mismatch"  << std::endl;
        std::cerr <<  node_counter <<  " " <<  nmbNodes  << std::endl;
        exit(0);
    }


    G.finish_construction();
    return 0;
}

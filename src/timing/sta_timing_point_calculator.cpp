/*
 * sta_timing_point_calculator.cpp
 *
 *  Created on: Mar 22, 2016
 *      Author: csguth
 */

#include "sta_timing_point_calculator.h"

#include <cmath>

namespace openeda {
namespace timing {

sta_timing_point_calculator::sta_timing_point_calculator(const graph & g):
    m_graph(g),
    m_counter(g.G()){
    reset_counter();
}

sta_timing_point_calculator::~sta_timing_point_calculator() {
}

void sta_timing_point_calculator::reset_counter()
{
    for(lemon::ListDigraph::NodeIt node(m_graph.G()); node != lemon::INVALID; ++node)
        m_counter[node]=0;
}

void sta_timing_point_calculator::push(lemon::ListDigraph::Node node) {
    assert(std::find(m_to_process.begin(), m_to_process.end(), node) == m_to_process.end());
    if(++m_counter[node] >= lemon::countInArcs(m_graph.G(), node))
        m_to_process.push_back(node);
}

void sta_timing_point_calculator::process_queue(const graph & g,  const graph_arcs_timing & arcs_timing, graph_nodes_timing & nodes_timing, lemon::ListDigraph::ArcMap< sta_arc_calculator * > & tarcs) {
    using namespace boost::units;
        for (std::deque<lemon::ListDigraph::Node>::const_iterator node_it = m_to_process.begin(); node_it != m_to_process.end(); ++node_it) {
            if(lemon::countInArcs(g.G(), *node_it) > 0)
            {
                quantity<si::time> arrival(std::numeric_limits<double>::lowest() * si::second);
                quantity<si::time> slew(std::numeric_limits<double>::lowest() * si::second);
                for (lemon::ListDigraph::InArcIt it(g.G(), *node_it); it != lemon::INVALID; ++it) {
                    lemon::ListDigraph::Node from(g.edge_source(static_cast<lemon::ListDigraph::Arc>(it)));
                    arrival = std::max(arrival, nodes_timing.arrival(from) + arcs_timing.delay(static_cast<lemon::ListDigraph::Arc>(it)));
                    slew = std::max(slew, arcs_timing.slew(static_cast<lemon::ListDigraph::Arc>(it)));
                }
                nodes_timing.arrival(*node_it, arrival);
                nodes_timing.slew(*node_it, slew);
            }
        }
    // CRITICAL >>>>
    for (auto node : m_to_process) {
        for (lemon::ListDigraph::OutArcIt it(g.G(), node); it != lemon::INVALID; ++it) {
            tarcs[it]->push(it);
        }
    }
    /// <<<<
    m_to_process.clear();

}

bool sta_timing_point_calculator::empty() {
    return m_to_process.empty();
}

} /* namespace timing */
} /* namespace openeda */


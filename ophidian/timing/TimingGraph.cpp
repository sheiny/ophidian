#include "TimingGraph.h"

namespace ophidian
{
namespace timing
{


TimingGraph::TimingGraph(const circuit::Netlist & netlist) :
    mPins(mGraph),
    mNodeProperties(mGraph),
    mArcs(mGraph),
    mArcProperties(mGraph),
    mRiseNodes(netlist.makeProperty<NodeType>(circuit::Pin())),
    mFallNodes(netlist.makeProperty<NodeType>(circuit::Pin()))
{
}

TimingGraph::~TimingGraph()
{

}

const TimingGraph::GraphType & TimingGraph::graph()
{
    return mGraph;
}

size_t TimingGraph::size(NodeType)
{
    return lemon::countNodes(mGraph);
}

size_t TimingGraph::size(ArcType)
{
    return lemon::countArcs(mGraph);
}


TimingGraph::NodeType TimingGraph::nodeCreate(const circuit::Pin & pin, const NodeProperty & prop, entity_system::Property<circuit::Pin, NodeType> & map)
{
    NodeType newNode;

    if (mGraph.valid(map[pin]))
        if (mPins[map[pin]] == pin)
            newNode = map[pin];
        else
            newNode = mGraph.addNode();
    else
        newNode = mGraph.addNode();

    map[pin] = newNode;
    mPins[newNode] = pin;
    mNodeProperties[newNode] = prop;

    return newNode;
}

TimingGraph::NodeType TimingGraph::riseNodeCreate(const circuit::Pin & pin)
{
    return nodeCreate(pin, NodeProperty::Rise, mRiseNodes);
}

TimingGraph::NodeType TimingGraph::riseNode(const circuit::Pin & pin)
{
    return mRiseNodes[pin];
}

TimingGraph::NodeType TimingGraph::fallNodeCreate(const circuit::Pin & pin)
{
    return nodeCreate(pin, NodeProperty::Fall, mFallNodes);
}

TimingGraph::NodeType TimingGraph::fallNode(const circuit::Pin & pin)
{
    return mFallNodes[pin];
}

TimingGraph::ArcType TimingGraph::arcCreate(const NodeType & from, const NodeType & to, TimingArc timingArc)
{
    ArcType graphEdge = mGraph.addArc(from, to);
    mArcs[graphEdge] = *(static_cast<Arc*>(&timingArc));
    mArcProperties[graphEdge] = ArcProperty::TimingArc;

    return graphEdge;
}

TimingGraph::ArcType TimingGraph::arcCreate(const NodeType & from, const NodeType & to, circuit::Net net)
{
    ArcType graphEdge = mGraph.addArc(from, to);
    mArcs[graphEdge] = *(static_cast<Arc*>(&net));
    mArcProperties[graphEdge] = ArcProperty::Net;

    return graphEdge;
}

TimingGraph::NodeProperty TimingGraph::property(const NodeType & node)
{
    return mNodeProperties[node];
}

TimingGraph::ArcProperty TimingGraph::property(const ArcType & arc)
{
    return mArcProperties[arc];
}

TimingGraph::NodeType TimingGraph::source(const ArcType & arc)
{
    return mGraph.source(arc);
}

TimingGraph::NodeType TimingGraph::target(const ArcType & arc)
{
    return mGraph.target(arc);
}

TimingGraph::GraphType::OutArcIt TimingGraph::outArc(const NodeType & node)
{
    return GraphType::OutArcIt(mGraph, node);
}

TimingGraph::GraphType::InArcIt TimingGraph::inArc(const NodeType & node)
{
    return GraphType::InArcIt(mGraph, node);
}

circuit::Pin TimingGraph::entity(const NodeType & node)
{
    return mPins[node];
}

circuit::Net TimingGraph::entity(circuit::Net, const ArcType & arc)
{
    if (mArcProperties[arc] != ArcProperty::Net)
        throw std::logic_error("Incompatible conversion. It is not a net!");

    return mArcs[arc];
}

TimingArc TimingGraph::entity(TimingArc, const ArcType & arc)
{
    if (mArcProperties[arc] != ArcProperty::TimingArc)
        throw std::logic_error("Incompatible conversion. It is not a timing arc!");

    return mArcs[arc];
}

TimingGraph::Arc TimingGraph::entity(Arc, const ArcType & arc)
{
    return mArcs[arc];
}

} // timing
} // ophidian
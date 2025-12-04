#include "Edge.hpp"
#include "BasicBlock.hpp"
#include <sstream>

Edge::Edge(std::shared_ptr<BasicBlock> src,
           std::shared_ptr<BasicBlock> tgt,
           EdgeType t,
           const std::string &cond)
    : source(src), target(tgt), type(t), condition(cond) {}

std::string Edge::getTypeString() const
{
    switch (type)
    {
    case EdgeType::FALL_THROUGH:
        return "FALL_THROUGH";
    case EdgeType::TRUE_BRANCH:
        return "TRUE_BRANCH";
    case EdgeType::FALSE_BRANCH:
        return "FALSE_BRANCH";
    case EdgeType::LOOP_BACK:
        return "LOOP_BACK";
    case EdgeType::JUMP:
        return "JUMP";
    case EdgeType::RETURN:
        return "RETURN";
    default:
        return "UNKNOWN";
    }
}

std::string Edge::toString() const
{
    std::stringstream ss;
    ss << "Edge: Block #" << source->getId()
       << " → Block #" << target->getId()
       << " [" << getTypeString() << "]";

    if (!condition.empty())
    {
        ss << " cond: " << condition;
    }

    return ss.str();
}
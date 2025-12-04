#pragma once
#include <memory>
#include <string>

class BasicBlock; // Forward declaration

// Define EdgeType enum HERE (not forward declared)
enum class EdgeType
{
    FALL_THROUGH, // Normal sequential flow: block A → block B
    TRUE_BRANCH,  // If condition true: if(cond) → then_block
    FALSE_BRANCH, // If condition false: if(cond) → else_block
    LOOP_BACK,    // Loop back edge: loop_end → loop_header
    JUMP,         // Unconditional jump (goto)
    RETURN        // Return from function
};

class Edge
{
private:
    std::shared_ptr<BasicBlock> source;
    std::shared_ptr<BasicBlock> target;
    EdgeType type;
    std::string condition;

public:
    Edge(std::shared_ptr<BasicBlock> src,
         std::shared_ptr<BasicBlock> tgt,
         EdgeType t,
         const std::string &cond = "");

    std::shared_ptr<BasicBlock> getSource() const { return source; }
    std::shared_ptr<BasicBlock> getTarget() const { return target; }
    EdgeType getType() const { return type; }
    const std::string &getCondition() const { return condition; }

    std::string getTypeString() const;
    std::string toString() const;
};
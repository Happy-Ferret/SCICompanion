#include "stdafx.h"
#include "CFGNode.h"
#include "TarjanAlgorithm.h"

using namespace std;

template<typename _Func>
void SomeFunction(DominatorMap &dominators, NodeSet &newDominators, CFGNode *n, _Func func)
{
    newDominators.insert(n);

    if (!func(n).empty())
    {
        NodeSet result;
        bool first = true;
        for (CFGNode *pred : func(n))
        {
            if (first)
            {
                result = dominators[pred];
                first = false;
            }
            else
            {
                NodeSet resultTemp;
                NodeSet &temp = dominators[pred];
                set_intersection(result.begin(), result.end(),
                    temp.begin(), temp.end(),
                    inserter(resultTemp, resultTemp.begin()));

                swap(resultTemp, result);
            }
        }

        for (CFGNode *node : result)
        {
            newDominators.insert(node);
        }
    }
    // Now look at all the predecessors p of n. 
    // Add in the intersection of all those dominators for them.
}

template<typename _Func>
DominatorMap GenerateDominators(NodeSet N, CFGNode *n0, _Func func)
{
    NodeSet N_minus_n0 = N;
    N_minus_n0.erase(n0);

    DominatorMap dominators;

    // dominator of the start node is the start itself
    dominators[n0].insert(n0);

    // for all other nodes, set all nodes as the dominators
    for (CFGNode *n : N_minus_n0)
    {
        dominators[n] = N;
    }

    // Iteratively eliminate nodes that are not dominators
    bool changes = true;
    while (changes)
    {
        changes = false;
        for (CFGNode *n : N_minus_n0)
        {
            NodeSet newDominatorsFor_n;
            SomeFunction(dominators, newDominatorsFor_n, n, func);
            if (newDominatorsFor_n != dominators[n])
            {
                dominators[n] = newDominatorsFor_n;
                changes = true;
            }
        }
    }

    return dominators;
}

// Filter out predecessors that are a single jump with no predecessors
// We need them in the tree because they might represent breaks in ifs, but
// they mess up the dominator tree and cause loops to not be identified.
NodeSet NoJmpPreds(CFGNode *node)
{
    NodeSet filtered;
    for (CFGNode *pred : node->Predecessors())
    {
        if (pred->Type == CFGNodeType::RawCode)
        {
            RawCodeNode *rawCodeNode = static_cast<RawCodeNode*>(pred);
            if (rawCodeNode->start->get_opcode() == Opcode::JMP)
            {
                if (rawCodeNode->Predecessors().empty())
                {
                    continue; // Don't insert it...
                }
            }
        }
        filtered.insert(pred);
    }
    return filtered;
}

DominatorMap GenerateDominators(NodeSet N, CFGNode *n0)
{
    return GenerateDominators(N, n0, [](CFGNode *node) { return NoJmpPreds(node); });
}


DominatorMap GeneratePostDominators(NodeSet N, CFGNode *n0)
{
    return GenerateDominators(N, n0, [](CFGNode *node) { return node->Successors(); });
}

bool IsReachable(CFGNode *head, CFGNode *tail)
{
    if (head == tail)
    {
        return true;
    }

    NodeSet visited;
    visited.insert(tail);

    stack<CFGNode*> toProcess;
    toProcess.push(tail);
    while (!toProcess.empty())
    {
        CFGNode *node = toProcess.top();
        toProcess.pop();

        for (CFGNode *pred : node->Predecessors())
        {
            if (pred == head)
            {
                return true;
            }

            if (visited.find(pred) == visited.end())
            {
                visited.insert(pred);
                toProcess.push(pred);
            }
        }
    }
    return false;
}

// Returns the set of all nodes between tail and head, assuming 
// head and tail form a single entry and exit point.
// Possible is there just for debugging. There should be no nodes outside possible
NodeSet CollectNodesBetween(CFGNode *head, CFGNode *tail, NodeSet possible)
{
    NodeSet collection;
    stack<CFGNode*> toProcess;
    toProcess.push(tail);
    while (!toProcess.empty())
    {
        CFGNode *node = toProcess.top();
        toProcess.pop();

        collection.insert(node);
        if (node != head)
        {
            for (CFGNode *pred : node->Predecessors())
            {
                if (collection.find(pred) == collection.end()) // Haven't already visited it
                {
                    assert(possible.find(pred) != possible.end());// We could just filter these, but let's assert for now to catch bad callers.
                    toProcess.push(pred);
                }
            }
        }
    }
    return collection;
}

map<CFGNode*, CFGNode*> CalculateImmediateDominators(const DominatorMap &dominatorMap)
{
    map<CFGNode*, CFGNode*> immediateDominators;
    for (const auto &pair : dominatorMap)
    {
        CFGNode *dominatee = pair.first;
        // One of its dominators is its immediate dominator
        // Its immediate dominator is the dominator that dominates it, but does not dominate any other
        // node that dominates the dominatee.
        // So this algorithm, I guess, is n^2 in complexity with the number of dominators for a node.
        // (If we care about perf, we could find the guy with the highest address and it would likely be the imm dom)
        const NodeSet &dominators = pair.second;

        for (CFGNode *potentialImmDom : dominators)
        {
            if (potentialImmDom != dominatee)   // I forget if dominator map for the dominatee includes the dominatee
            {
                bool found = true;
                for (CFGNode *test : dominators)
                {
                    if ((test != potentialImmDom) && (test != dominatee))
                    {
                        if (dominatorMap.at(test).find(potentialImmDom) != dominatorMap.at(test).end())
                        {
                            // Our potential immediate dominator dominates other nodes of the dominatees dominators.
                            // So it can't be the immediate dominator
                            found = false;
                            break;
                        }
                    }
                }
                if (found)
                {
                    immediateDominators[dominatee] = potentialImmDom;
                    break;
                }
            }
        }
    }

    assert((immediateDominators.size() == (dominatorMap.size() - 1)) && "Every node must have an immediate dominator except the header");
    return immediateDominators;
}

map<CFGNode*, CFGNode*> CalculateImmediatePostDominators(const DominatorMap &postDominators)
{
    return CalculateImmediateDominators(postDominators);
}
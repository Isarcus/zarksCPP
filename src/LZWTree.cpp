#include <zarks/io/LZWTree.h>

namespace zmath
{

LZWTree::LZWNode::LZWNode(size_t code)
    : code(code)
{}

LZWTree::LZWTree(size_t startCodes)
    : root(new LZWNode(npos))
    , current(root)
    , size(startCodes)
{
    for (size_t i = 0; i < startCodes; i++)
    {
        root->nodes[i] = new LZWNode(i);
    }
}

LZWTree::~LZWTree()
{
    recursiveDelete(root);
    root = nullptr;
    current = nullptr;
    size = 0;
}

size_t LZWTree::Add(size_t nextIdx)
{
    // Get iterator to queried next index
    auto iter = current->nodes.find(nextIdx);

    if (iter == current->nodes.end())
    {
        // If the next index IS NOT found, add it to the tree and
        // return the value that represents the index buffer so far.
        current->nodes[nextIdx] = new LZWNode(size++);
        size_t retCode = current->code;
        current = root->nodes.at(nextIdx);
        return retCode;
    }
    else
    {
        // If the next index IS found, just advance the index buffer,
        // but return npos to signify that the code stream should not
        // be appended to yet.
        current = iter->second;
        return npos;
    }
}

void LZWTree::Reset(size_t startCodes)
{
    size = startCodes;

    // Delete all of root's nodes
    for (auto& pair : root->nodes)
    {
        recursiveDelete(pair.second);
    }
    root->nodes.clear();

    // Give root the designated number of starting nodes
    for (size_t i = 0; i < startCodes; i++)
    {
        root->nodes[i] = new LZWNode(i);
    }
}

size_t LZWTree::Size() const
{
    return size;
}

void LZWTree::recursiveDelete(LZWNode* node)
{
    for (auto& pair : node->nodes)
    {
        recursiveDelete(pair.second);
    }

    delete node;
}

} // namespace zmath

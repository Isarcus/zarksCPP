#pragma once

#include <unordered_map>

namespace zmath
{
    class LZWTree
    {
    public:
        // Construct a new LZW encoding tree.
        // @param startCodes the number of child nodes with which to
        //        initialize the root node. No value in the index
        //        stream may exceed this number.
        LZWTree(size_t startCodes = 0);
        LZWTree(const LZWTree& tree) = delete;
        LZWTree(LZWTree&& tree) = delete;
        ~LZWTree();

        // Add a new index to the tree.
        // @return npos if a code for the current sequence of indices
        //         already exists, otherwise the value of the code
        //         to append to the code stream.
        size_t Add(size_t nextIdx);

        // Reset the tree as if it had just been constructed with
        // LZWTree(startCodes).
        void Reset(size_t startCodes = 0);

        // @return The number of codes in the tree.
        size_t Size() const;

        // The maximum possible representable code.
        static constexpr size_t npos = -1;

    private:
        struct LZWNode
        {
            LZWNode(size_t code);

            size_t code;
            std::unordered_map<size_t, LZWNode*> nodes;
        };

        LZWNode* root;
        LZWNode* current;
        size_t size;

        static void recursiveDelete(LZWNode* node);
    };

} // namespace zmath

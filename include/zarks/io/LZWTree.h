#pragma once

#include <unordered_map>

namespace zmath
{
    class LZWTree
    {
    public:
        LZWTree(size_t startCodes = 0);
        LZWTree(const LZWTree& tree) = delete;
        LZWTree(LZWTree&& tree) = delete;
        ~LZWTree();

        size_t Add(size_t nextIdx);
        void Reset(size_t startCodes = 0);

        size_t Size() const;

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

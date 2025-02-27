#pragma once

namespace opencmd {

    class ChainNode;

    class ChainAccess {
    public:
        virtual ~ChainAccess() = default;

        // This is the only method that can be invoked by a generic node 
        // in order to access the different part of the tree
        virtual const std::shared_ptr<ChainNode> getNode(const std::string& name) const = 0;
    };

}
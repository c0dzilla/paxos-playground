#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include "node.h"

namespace paxos {

  class NodeRegistry {

    public:
     static std::shared_ptr<Node> Register(int nodeId);

     static std::unordered_set<std::shared_ptr<Node>> Get();

     // This method will only return a non-null value if all nodes reported the
     // same committed value. This assumes network / node availability for all.
     static std::shared_ptr<std::string> GetCommittedValue();

    private:
     static std::unordered_set<std::shared_ptr<Node>> nodes_;
  };
}

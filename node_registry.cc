#include "node_registry.h"

using ::std::make_shared;
using ::std::shared_ptr;
using ::std::string;
using ::std::unordered_set;

namespace paxos {

  unordered_set<shared_ptr<Node>> NodeRegistry::nodes_;

  shared_ptr<Node> NodeRegistry::Register(int nodeId) {
    const auto& node = make_shared<Node>(nodeId);
    nodes_.insert(node);
    return node;
  }


  unordered_set<shared_ptr<Node>> NodeRegistry::Get() {
    return nodes_;
  }


  shared_ptr<string> NodeRegistry::GetCommittedValue() {
    shared_ptr<string> committed_value = nullptr;

    for (const auto& node : nodes_) {
      const auto& node_committed_value = node->GetCommittedValue();

      if (!node_committed_value) {
        return nullptr;
      }

      if (!committed_value) {
        committed_value = node_committed_value;
      } else if (*committed_value != *node_committed_value) {
        return nullptr;
      }
    }

    return committed_value;
  }

}
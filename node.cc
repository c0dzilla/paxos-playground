#include <iostream>
#include <vector>
#include "node.h"

using ::std::cout;
using ::std::make_pair;
using ::std::make_shared;
using ::std::shared_ptr;
using ::std::string;
using ::std::vector;

namespace paxos {

  Node::Node(vector<shared_ptr<Node>> nodes, int node_id) {
    nodes_ = nodes;
    node_id_ = node_id;
    generation_clock_val_ = GenerationClock::Get();
    cout << generation_clock_val_ << std::endl;
  }

  void Node::SetProposalValue(string value) {
    if (!proposal_) {
      proposal_ = make_shared<Proposal>();
    }
    proposal_->generation = GetGeneration();
    proposal_->value = value;
  }

  void Node::Propose() {
    if (!proposal_) {
      return;
    }
    // Begin PROPOSE PHASE.
    const int quoram = nodes_.size() / 2 + 1;
    vector<shared_ptr<Node>> acceptedNodes;
    Generation highestAcceptedGeneration = make_pair(INT_MIN, INT_MIN);
    string potentialValueToAdopt;

    for (const auto& node : nodes_) {
      // Stub the networking layer.
      const auto& promise = node->HandleProposal(shared_from_this(),
          *proposal_);

      if (promise->accepted) {
        acceptedNodes.push_back(node);

        if (promise->acceptReq
            && promise->acceptReq->generation > highestAcceptedGeneration) {
          highestAcceptedGeneration = promise->acceptReq->generation;
          potentialValueToAdopt = promise->acceptReq->value;
        }

      }
    }

    // Achieved quoram. Move to ACCEPT PHASE.
    if (acceptedNodes.size() > quoram) {
      if (highestAcceptedGeneration > GetGeneration())
      // Update my value to match the largest generation that was accepted.
      SetProposalValue(potentialValueToAdopt);

      AcceptReq acceptReq;
      acceptReq.generation = GetGeneration();
      acceptReq.value = proposal_->value;
      SendAcceptReqs(acceptedNodes, acceptReq);
    }
  }

  shared_ptr<Promise> Node::HandleProposal(shared_ptr<Node> proposer,
      Proposal proposal) {
    return nullptr;
  }

  void Node::SendAcceptReqs(vector<shared_ptr<Node>> nodes,
      AcceptReq acceptReq) {
    for (const auto& acceptedNode : nodes) {
      // Stub the networking layer.
      acceptedNode->HandleAcceptReq(shared_from_this(), acceptReq);
    }
  }

  shared_ptr<AcceptResponse> Node::HandleAcceptReq(
      shared_ptr<Node> acceptRequester, AcceptReq acceptReq) {
    return nullptr;
  }

}

int main() {
  paxos::Node node1({}, 1);
  paxos::Node node2({}, 1);
  paxos::Node node3({}, 1);
  paxos::Node node4({}, 1);

  return 0;
}

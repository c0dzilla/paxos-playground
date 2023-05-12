#include <assert.h>
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

  Node::Node(int node_id) {
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
    const auto& nodes =  NodeRegistry::Get();
    const int quoram = nodes.size() / 2 + 1;
    vector<shared_ptr<Node>> acceptedNodes;
    Generation highestAcceptedGeneration = make_pair(INT_MIN, INT_MIN);
    string potentialValueToAdopt;

    for (const auto& node : nodes) {
      // Stub the networking layer.
      const auto& promise = node->HandleProposal(shared_from_this(),
          *proposal_);

      if (!promise) {
        continue;
      }

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
    if (acceptedNodes.size() >= quoram) {
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
    if (!promised_generation_) {
      return make_shared<Promise>(true, nullptr);
    }

    if (proposal.generation > *promised_generation_) {
      *promised_generation_ = proposal.generation;

      if (accepted_req_) {
        return make_shared<Promise>(true, accepted_req_);
      }
      return make_shared<Promise>(true, nullptr);
    }

    return make_shared<Promise>(false, nullptr);
  }


  void Node::SendAcceptReqs(vector<shared_ptr<Node>> nodes,
      AcceptReq acceptReq) {
    const int quoram = nodes.size() / 2 + 1;
    int acceptedCount = 0;

    for (const auto& acceptedNode : nodes) {
      // Stub the networking layer.
      const auto& acceptResponse =
          acceptedNode->HandleAcceptReq(shared_from_this(), acceptReq);

      if (acceptResponse && acceptResponse->accepted) {
        acceptedCount++;
      }
    }

    if (acceptedCount >= quoram) {
      // Send this proposal as a COMMIT to all nodes.
      assert(proposal_ != nullptr);

      for (const auto& node : NodeRegistry::Get()) {
        node->Commit(proposal_->value);
      }
    }
  }


  shared_ptr<AcceptResponse> Node::HandleAcceptReq(
      shared_ptr<Node> acceptRequester, AcceptReq acceptReq) {
    // Accept the request if its generation is >= the highest generation that
    // has been promised so far. Theoretically, checking for = should be
    // sufficient, since the accept request's generation cannot be higher than
    // the highest promised generation.
    if (!promised_generation_
        || acceptReq.generation >= *promised_generation_) {

      if (!accepted_req_) {
        accepted_req_ = make_shared<AcceptReq>();
      }
      *accepted_req_ = acceptReq;
      return make_shared<AcceptResponse>(true);
    }

    return make_shared<AcceptResponse>(false);
  }

  void Node::Commit(string value) {
    if (!committed_value_) {
      committed_value_ = make_shared<string>(value);
    } else {
      *committed_value_ = value;
    }
  }

}


int main() {
  const auto& node1 = paxos::NodeRegistry::Register(1);
  const auto& node2 = paxos::NodeRegistry::Register(2);
  const auto& node3 = paxos::NodeRegistry::Register(3);
  const auto& node4 = paxos::NodeRegistry::Register(4);
  const auto& node5 = paxos::NodeRegistry::Register(5);

  return 0;
}

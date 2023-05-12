#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "node.h"
#include "node_registry.h"

using ::std::atomic_int;
using ::std::chrono::seconds;
using ::std::cout;
using ::std::endl;
using ::std::make_pair;
using ::std::make_shared;
using ::std::shared_ptr;
using ::std::string;
using ::std::this_thread::sleep_for;
using ::std::thread;
using ::std::to_string;
using ::std::vector;

namespace paxos {

  atomic_int GenerationClock::epoch_{0};

  Node::Node(int node_id) {
    node_id_ = node_id;
    generation_clock_val_ = GenerationClock::Get();
    cout << "Created Node with id: " << node_id <<
        " , Generation Clock value: " << generation_clock_val_ << std::endl;
  }


  string Node::ToString() {
    if (proposal_) {
      return "{ node_id: " + to_string(node_id_) + ", proposed_value: "
          + proposal_->value + " }";
    }
    return "{ node_id: " + to_string(node_id_) + " }";
  }


  void Node::SetProposalValue(string value) {
    if (!proposal_) {
      proposal_ = make_shared<Proposal>();
    }
    proposal_->generation = GetGeneration();
    proposal_->value = value;
  }


  void Node::Propose() {
    // Endlessly loop until if have a proposal for the cluster and have not yet
    // committed to a value.
    while (proposal_ && !committed_value_) {
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
        cout << this->ToString() << " PROPOSED " << node->ToString() << endl;

        if (!promise) {
          continue;
        }

        if (promise->accepted) {
          cout << node->ToString() << " PROMISED " << this->ToString() << endl;
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
        cout << this->ToString()
            << " achieved quoram in PROPOSE phase. Moving to ACCEPT phase"
            << endl;

        if (highestAcceptedGeneration > GetGeneration())
        // Update my value to match the largest generation that was accepted.
        SetProposalValue(potentialValueToAdopt);

        AcceptReq acceptReq;
        acceptReq.generation = GetGeneration();
        acceptReq.value = proposal_->value;
        SendAcceptReqs(acceptedNodes, acceptReq);
      }
    }
  }


  shared_ptr<Promise> Node::HandleProposal(shared_ptr<Node> proposer,
      Proposal proposal) {
    // Simulate random network delay.
    sleep_for(seconds(GenerationClock::Get() % 7));
    // Simulate random network partition.
    if (GenerationClock::Get() % 11 == 0) {
      return nullptr;
    }

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
      cout << this->ToString() << " requested ACCEPT from "
          << acceptedNode->ToString() << endl;

      if (acceptResponse && acceptResponse->accepted) {
        cout << acceptedNode->ToString() << " ACCEPTED " << this->ToString()
            << endl;
        acceptedCount++;
      }
    }

    if (acceptedCount >= quoram) {
      // Send the accepted value as a COMMIT to all nodes.
      for (const auto& node : NodeRegistry::Get()) {
        node->Commit(acceptReq.value);
      }
    }
  }


  shared_ptr<AcceptResponse> Node::HandleAcceptReq(
      shared_ptr<Node> acceptRequester, AcceptReq acceptReq) {
    // Simulate random network delay.
    sleep_for(seconds(GenerationClock::Get() % 7));
    // Simulate random network partition.
    if (GenerationClock::Get() % 11 == 0) {
      return nullptr;
    }
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


  shared_ptr<string> Node::GetCommittedValue() {
    return committed_value_;
  }

}


int main() {
  const auto& node1 = paxos::NodeRegistry::Register(1);
  const auto& node2 = paxos::NodeRegistry::Register(2);
  const auto& node3 = paxos::NodeRegistry::Register(3);
  const auto& node4 = paxos::NodeRegistry::Register(4);
  const auto& node5 = paxos::NodeRegistry::Register(5);

  thread consensusChecker([] {
    shared_ptr<string> committedValue = nullptr;

    while (!committedValue) {
      cout << "Paxos consensus not achieved yet..." << endl;
      sleep_for(seconds(2));
      committedValue = paxos::NodeRegistry::GetCommittedValue();
    }
    cout << "Paxos consensus achieved; value committed: " << *committedValue
        << endl;
  });

  thread proposer1([node1] {
    node1->SetProposalValue("alice");
    node1->Propose();
  });

  thread proposer2([node2] {
    node2->SetProposalValue("bob");
    node2->Propose();
  });

  consensusChecker.join();
  proposer1.join();
  proposer2.join();

  return 0;
}

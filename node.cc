#include <assert.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include "node.h"
#include "node_registry.h"

using ::std::atomic_int;
using ::std::chrono::seconds;
using ::std::cout;
using ::std::endl;
using ::std::lock_guard;
using ::std::make_pair;
using ::std::make_shared;
using ::std::mutex;
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
      return "{ node_id: " + to_string(node_id_) + ", generation_clock_val: "
          + to_string(generation_clock_val_) + ", proposed_value: "
              + proposal_->value + " }";
    }
    return "{ node_id: " + to_string(node_id_) + ", generation_clock_val: "
        + to_string(generation_clock_val_)  + " }";
  }


  void Node::SetProposalValue(string value) {
    if (!proposal_) {
      proposal_ = make_shared<Proposal>();
    }
    proposal_->generation = GetGeneration();
    proposal_->value = value;
  }


  void Node::RefreshProposal() {
    if (proposal_) {
      generation_clock_val_ = GenerationClock::Get();
      proposal_->generation = GetGeneration();
    }
  }


  void Node::Propose() {
    // Endlessly loop until if have a proposal for the cluster and have not yet
    // committed to a value.
    while (proposal_ && !committed_value_) {
      // Obtain a fresh generation before every PROPOSAL attempt.
      RefreshProposal();

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

        if (!promise || !promise->accepted) {
          cout << node->ToString() << " DID NOT PROMISE " << this->ToString()
              << endl;
          continue;
        }
        cout << node->ToString() << " PROMISED " << this->ToString() << endl;
        acceptedNodes.push_back(node);

        if (promise->acceptReq
            && promise->acceptReq->generation > highestAcceptedGeneration) {
          highestAcceptedGeneration = promise->acceptReq->generation;
          potentialValueToAdopt = promise->acceptReq->value;
        }
      }

      // Achieved quoram. Move to ACCEPT PHASE.
      if (acceptedNodes.size() >= quoram) {
        cout << this->ToString()
            << " achieved quoram in PROPOSE phase. Moving to ACCEPT phase..."
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
    // Obtain mutex so that PROPOSAL processing is serialized.
    const lock_guard<mutex> lock(handle_proposal_mutex_);

    // Simulate random network delay.
    sleep_for(seconds(GenerationClock::Get() % 7));

    // Simulate random network partition.
    if (GenerationClock::Get() % 11 == 0) {
      return nullptr;
    }

    if (committed_value_) {
      // Already committed to a value, so cannot PROMISE.
      return nullptr;
    }

    if (!promised_generation_) {
      promised_generation_ = make_shared<Generation>();
      *promised_generation_ = proposal.generation;
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

    for (const auto& promisedNode : nodes) {
      // Stub the networking layer.
      const auto& acceptResponse =
          promisedNode->HandleAcceptReq(shared_from_this(), acceptReq);
      cout << this->ToString() << " requested ACCEPT from "
          << promisedNode->ToString() << endl;

      if (acceptResponse && acceptResponse->accepted) {
        cout << promisedNode->ToString() << " ACCEPTED " << this->ToString()
            << endl;
        acceptedCount++;
      } else {
        cout << promisedNode->ToString() << " DID NOT ACCEPT "
            << this->ToString() << endl;
      }
    }

    if (acceptedCount >= quoram) {
      cout << this->ToString()
          << " achieved quoram in ACCEPT phase. Sending COMMIT to all nodes..."
          << endl;
      // Send the accepted value as a COMMIT to all nodes.
      for (const auto& node : NodeRegistry::Get()) {
        node->Commit(acceptReq.value);
      }
    }
  }


  shared_ptr<AcceptResponse> Node::HandleAcceptReq(
      shared_ptr<Node> acceptRequester, AcceptReq acceptReq) {
    // Obtain mutex so that ACCEPT processing is serialized.
    const lock_guard<mutex> lock(handle_accept_req_mutex_);

    // Simulate random network delay.
    sleep_for(seconds(GenerationClock::Get() % 7));

    // Simulate random network partition.
    if (GenerationClock::Get() % 11 == 0) {
      return nullptr;
    }

    if (committed_value_) {
      // Already committed to a value, so cannot ACCEPT.
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
      // Paxos consensus not achieved yet.
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

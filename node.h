#include <atomic>
#include <climits>
#include <memory>
#include <string>
#include <vector>

namespace paxos {

  typedef std::pair<int, int> Generation;

  bool operator<(const Generation& g1, const Generation& g2) {
    if (g1.first == g2.first) {
      return g1.second > g2.second;
    }
    return g1.first > g2.first;
  }

  struct Proposal {
    Generation generation;
    std::string value;
  };

  struct AcceptReq {
    Generation generation;
    std::string value;
  };

  struct Promise {
    bool accepted;
    std::shared_ptr<AcceptReq> acceptReq = nullptr;
  };

  struct AcceptResponse {
    bool accepted;
  };

  class GenerationClock {
    public:
     static int Get() {
       return epoch.fetch_add(1);
     }

    private:
     static std::atomic_int epoch;
  };

  std::atomic_int GenerationClock::epoch{0};


  class Node : public std::enable_shared_from_this<Node> {

    public:
     Node(std::vector<std::shared_ptr<Node>> nodes, int node_id);

     void SetProposalValue(std::string value);

    private:
     Node();

     void Propose();

     std::shared_ptr<Promise> HandleProposal(std::shared_ptr<Node> proposer,
         Proposal proposal);

     void SendAcceptReqs(std::vector<std::shared_ptr<Node>> nodes,
        AcceptReq acceptReq);

     std::shared_ptr<AcceptResponse> HandleAcceptReq(
         std::shared_ptr<Node> acceptRequester, AcceptReq acceptReq);

     Generation GetGeneration() {
       return std::make_pair(generation_clock_val_, node_id_);
     }

    private:
     std::vector<std::shared_ptr<Node>> nodes_;

     int node_id_;

     int generation_clock_val_;

     std::shared_ptr<Proposal> proposal_ = nullptr;

     std::shared_ptr<Generation> promised_generation_ = nullptr;

     std::shared_ptr<AcceptReq> accepted_req_ = nullptr;
  };
}

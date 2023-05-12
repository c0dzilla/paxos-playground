#include <atomic>
#include <climits>
#include <memory>
#include <string>
#include <unordered_set>
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
    // The `value` is needed only in the AcceptReq, and not in the Proposal.
    // This is because the value is not read during the PROPOSE-PROMISE phase.
    // But for simplicity, let's keep AcceptReq and Proposal msgs similar.
    std::string value;
  };

  typedef Proposal AcceptReq;

  struct Promise {
    Promise(bool accepted, std::shared_ptr<AcceptReq> acceptReq) :
        accepted{accepted}, acceptReq{acceptReq} {}

    bool accepted;
    std::shared_ptr<AcceptReq> acceptReq = nullptr;
  };

  struct AcceptResponse {
    AcceptResponse(bool accepted) : accepted{accepted} {}
    bool accepted;
  };

  class GenerationClock {

    public:
     static int Get() {
       return epoch_.fetch_add(1);
     }

    private:
     static std::atomic_int epoch_;
  };

  std::atomic_int GenerationClock::epoch_{0};

  class Node : public std::enable_shared_from_this<Node> {

    public:
     Node(int node_id);

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

     void Commit(std::string value);

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

     std::shared_ptr<std::string> committed_value_ = nullptr;
  };

  class NodeRegistry {

    public:
     static std::shared_ptr<Node> Register(int nodeId) {
      const auto& node = std::make_shared<Node>(nodeId);
       nodes_.insert(node);
       return node;
     }

     static std::unordered_set<std::shared_ptr<Node>> Get() {
      return nodes_;
     }

    private:
     static std::unordered_set<std::shared_ptr<Node>> nodes_;
  };

  std::unordered_set<std::shared_ptr<Node>> NodeRegistry::nodes_;
}

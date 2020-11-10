// Last edited: 1st Nov. by A.Zaliwski  License: MIT

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

CONTRACT fredivregist : public contract {
  public:
    using contract::contract;

    ACTION proposalclr();                   // erase current proposal

    ACTION allowance( uint64_t key, name vipname ); // creating whitelist of allowed proposers, voters.

    ACTION proposalnew(                     // Proposer entered parameters
        name proposer,
        name vaccount,                      //!< DAPP account (if any)
        name eosaccount,                    //!< freeos account used to receive dividends and for identification
        char policy_name,                   //!< policy_name - (a)WayFarer or (b)WayFinder
        char user_type,                     //!< user_type -(f)ounder or (i)nvestor
        uint64_t usd_investment,
        uint32_t target_freeos_price,       //TODO verify types
        uint8_t roi_target_cap,
        uint8_t weekly_percentage,
        bool locked,                        //!< lock dividends for new members (option). Note: When unlock cannot be locked again. (TODO need function)
        uint64_t rates_left                 // number of payments left under this policy (apply only to specific variants)
      );

  private:

         // one hour in seconds (Computation: 60 minutes * 60 seconds)
        constexpr static uint32_t EXPIRATION_PERIOD = 60 * 60;

        static inline time_point_sec current_time_point_sec() {
            return time_point_sec(current_time_point());
        }

TABLE proposal_struct {                     // Note: release RAM after approval
        uint64_t key;                       // primary key
        name eosaccount;                    //!< freeos account used to receive dividends and for identification
        char policy_name;                   //!< policy_name - (a) WayFarer or (b) WayFinder
        char user_type;                     //!< user_type -(f)ounder or (i)nvestor
        uint64_t usd_investment;
        uint32_t target_freeos_price;
        uint8_t roi_target_cap;             //percentage
        uint8_t weekly_percentage;          //percentage
        bool locked;                        //!< lock dividends for selected new members. Note: When unlock cannot be locked again.
        time_point_sec expires_at;          //!< expiry for the proposal only (must be voted before that time)
        uint64_t rates_left;                //Initial number of payments (count down)
        uint64_t primary_key() const { return key; }
        };

        typedef eosio::multi_index<"proposals"_n, proposal_struct> proposal_table;
        bool is_expired() const { return current_time_point_sec() >= expires_at; }   //does it work? TODO

TABLE register_struct {
      //Note: These two accounts also determine ownership for giving or NFT seling transaction (if any)
      name     eosaccount;                  //!< freeos account used to receive dividends and for identification
      char     policy_name;                 //!< policy_name - (a) WayFarer or (b) WayFinder
      char     user_type;                   //!< user_type -(f)ounder or (i)nvestor
      uint64_t usd_investment;
      uint32_t target_freeos_price;
      uint8_t  roi_target_cap;
      uint8_t  weekly_percentage;
      //Note: This register record is single NFT by itself
      uint64_t mint_date;                   // current date (when this register record was created)
      bool     locked;                      //!< lock dividends for selected new members. Note: When unlock should be not lock again.
      uint64_t rates_left;                  // number of payments left under this policy (apply only to specific variants)
      uint32_t sharefraction;               // which fraction of a total pool that person receives (optional for consideration).
      uint64_t primary_key()const {return eosaccount.value;}
    };

    typedef eosio::multi_index<"registers"_n, register_struct> register_table;

//
TABLE whitelist_struct {                  //!< whitelist_struct - Contains list of allowed proposers and voters along with their vote.
      name     user;                      //!< First user is a proposer, the others are voters
      uint8_t  vote;                      //!< 0 -n/a  1 - no   2 - yes /     - different than zero if already voted
      uint64_t primary_key() const { return user.value; }   //!< This table should be filled up only by a real multisig
    };
      using whitelist_index = eosio::multi_index<"whitelist"_n, whitelist_struct>;

TABLE status_messages { //Keep the error numbers list for the last proposal to read by frontend. New proposal erase the list.
      uint64_t key;
      uint8_t  errorno;
      auto primary_key() const { return key; }
    };
    typedef multi_index<name("messages"), messages> messages_table;
};

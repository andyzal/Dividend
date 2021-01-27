// time limit for voting 
constexpr static uint32_t EXPIRATION_PERIOD = 60 * 60;    // one hour in seconds 

// copies of declarations from freeos333333/freeostokens for inline operations -
  struct [[eosio::table]] account {
    asset    balance;
    uint64_t primary_key()const { return balance.symbol.code().raw(); }
  };
//
  struct [[eosio::table]] currency_stats {
    asset    supply;
    asset    max_supply;
    name     issuer;
    uint64_t primary_key()const { return supply.symbol.code().raw(); }
  };
  typedef eosio::multi_index< "accounts"_n, account > accounts;
  typedef eosio::multi_index< "stat"_n, currency_stats > stats;
// end of extra declarations

// Account names used in the code: 
  const std::string freeos_acct = "freeos333333";
  const name tokencontra   = "freeos333333"_n; // Token contract (for inline transfers) - replaces eosio.token for inlines 
  const name profitaccount = "freeosdivide"_n; // Here is stored profit to divide.
  const name daoaccount    = "freeosdaodao"_n; // Organizational target DAO account?
#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>


namespace eosiosystem {
   class system_contract;
}

   using namespace eosio;
   using std::string;

enum registration_status{ registered_already,
                          registered_success,
                          };

   /**
    * @defgroup freeos freeos contract
    * @ingroup eosiocontracts
    *
    * freeos contract
    *
    * @details freeos contract defines the structures and actions that allow users to create, issue, and manage
    * tokens on eosio based blockchains.
    * @{
    */
   class [[eosio::contract("freeos")]] freeos : public contract {
      public:
         using contract::contract;

         /**
          * version action.
          *
          * @details Prints the version of this contract.
          */
         [[eosio::action]]
         void version();

         /**
          * tick action.
          *
          * @details Triggers scheduled actions.
          */
         [[eosio::action]]
         void tick();

         /**
          * hourly action.
          *
          * @details Triggers hourly scheduled actions.
          */
         [[eosio::action]]
         void hourly();

         /**
          * daily action.
          *
          * @details Triggers daily scheduled actions.
          */
         [[eosio::action]]
         void daily();

         /**
          * weekly action.
          *
          * @details Triggers weekly scheduled actions.
          */
         [[eosio::action]]
         void weekly();


         /**
          * reguser action.
          *
          * @details Creates a record for the user in the 'stakereqs' table.
          * @param user - the account to be registered,
          * @param account_type - the type of account: "e" is EOS wallet user, "d" is Dapp Account user, "v" is Voice verified user.
          *
          * @pre Requires permission of the contract account
          *
          * If validation is successful a new entry in 'stakereqs' table is created for the user account.
          */
         [[eosio::action]]
         void reguser(const name& user, const std::string account_type);

         // this action for maintenance purposes
         [[eosio::action]]
         void maintain(std::string option);

         /**
          * dereg action.
          *
          * @details Creates a record for the user in the 'stakereqs' table.
          * @param user - the account to be registered,
          * @param account_type - the type of account: "e" is EOS wallet user, "d" is Dapp Account user, "v" is Voice verified user.
          *
          * @pre Requires permission of the contract account
          * @pre The user account must be previously registered
          * @pre The account must not have any staked EOS recorded
          *
          * If validation is successful the record for the user account is deleted from the 'stakereqs' table.
          */
         [[eosio::action]]
         void dereg(const name& user);

         /**
          * stake action.
          *
          * @details Adds a record of staked EOS tokens in the user account record in the 'stakereqs' table.\n
          * @details This action is automatically invoked on the freeos account receiving EOS from the user.\n
          *
          * @param user - the account sending the EOS tokens to freeos,
          * @param to - the 'freeos' account,
          * @param quantity - the asset, i.e. an amount of EOS which is transferred to the freeos account,
          * @param memo - a memo describing the transaction
          *
          * @pre The user account must be previously registered
          * @pre The asset must be equal to the stake requirement for the user
          *
          * If validation is successful the record for the user account is updated to record the amount staked and the date/time of the receipt.
          */
         [[eosio::on_notify("eosio.token::transfer")]]
         void stake(name user, name to, asset quantity, std::string memo);

         /**
          * unstake action.
          *
          * @details Transfer staked EOS back to the user if the waiting time of 1 week has elapsed since staking.
          * @details Updates the record of staked EOS tokens in the user account record in the 'stakereqs' table.\n
          *
          * @param user - the account to transfer the previously EOS tokens to,
          *
          * @pre The user account must be previously registered and has a non-zero balance of staked EOS
          *
          * If transfer is successful the record for the user account is updated to set the amount staked and stake time/date to zero.
          */
         [[eosio::action]]
         void unstake(const name& user);

         /**
          * getuser action.
          *
          * @details For testing purposes, prints the values stored in the user account record in the 'stakereqs' table.
          *
          * @param user - the user account
          *
          * @pre The user account must be previously registered
          *
          */
         [[eosio::action]]
         void getuser(const name& user);

         /**
          * Create action.
          *
          * @details Allows `issuer` account to create a token in supply of `maximum_supply`.
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          *
          * If validation is successful a new entry in statstable for token symbol scope gets created.
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply);
         /**
          * Issue action.
          *
          * @details This action issues to `to` account a `quantity` of tokens.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quntity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          */
         [[eosio::action]]
         void issue( const name& to, const asset& quantity, const string& memo );

         /**
          * Retire action.
          *
          * @details The opposite for create action, if all validations succeed,
          * it debits the statstable.supply amount.
          *
          * @param quantity - the quantity of tokens to retire,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void retire( const asset& quantity, const string& memo );

         /**
          * Transfer action.
          *
          * @details Allows `from` account to transfer to `to` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
         /**
          * Open action.
          *
          * @details Allows `ram_payer` to create an account `owner` with zero balance for
          * token `symbol` at the expense of `ram_payer`.
          *
          * @param owner - the account to be created,
          * @param symbol - the token to be payed with by `ram_payer`,
          * @param ram_payer - the account that supports the cost of this action.
          *
          * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
          * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
          */
         [[eosio::action]]
         void open( const name& owner, const symbol& symbol, const name& ram_payer );

         /**
          * Close action.
          *
          * @details This action is the opposite for open, it closes the account `owner`
          * for token `symbol`.
          *
          * @param owner - the owner account to execute the close action for,
          * @param symbol - the symbol of the token to execute the close action for.
          *
          * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
          * @pre If the pair of owner plus symbol exists, the balance has to be zero.
          */
         [[eosio::action]]
         void close( const name& owner, const symbol& symbol );

         /**
          * claim action.
          *
          * @details This action is run by the user to claim this week's allocation of freeos tokens.
          *
          * @param owner - the user account to execute the claim action for.
          *
          * @pre Requires authorisation of the user account
          * @pre The user must pass claim eligibility requirements:
          * - must be registered as a freeos user (has a record in the users table)
          * - must not have already claimed for the current week
          * - must have staked the required amount of EOS tokens
          * - must hold the required amount of freeos tokens.
          */
         [[eosio::action]]
         void claim( const name& user);

         /**
          * getcounts action.
          *
          * @details This action is run by anyone to print the counts of users registered and claim events.
          *
          */
         [[eosio::action]]
         void getcounts();

         /**
          * Get supply method.
          *
          * @details Gets the supply for token `sym_code`, created by `token_contract_account` account.
          *
          * @param token_contract_account - the account to get the supply for,
          * @param sym_code - the symbol to get the supply for.
          */
         static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         /**
          * Get balance method.
          *
          * @details Get the balance for a token `sym_code` created by `token_contract_account` account,
          * for account `owner`.
          *
          * @param token_contract_account - the token creator account,
          * @param owner - the account for which the token balance is returned,
          * @param sym_code - the token for which the balance is returned.
          */
         static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }


         using create_action = eosio::action_wrapper<"create"_n, &freeos::create>;
         using issue_action = eosio::action_wrapper<"issue"_n, &freeos::issue>;
         using retire_action = eosio::action_wrapper<"retire"_n, &freeos::retire>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &freeos::transfer>;
         using open_action = eosio::action_wrapper<"open"_n, &freeos::open>;
         using close_action = eosio::action_wrapper<"close"_n, &freeos::close>;

      private:

         // record of last time timed processes were run - there is one record hence zero is returned as primary_key
         struct [[eosio::table]] ticker {
            time_point_sec  tickly;
            time_point_sec  hourly;
            time_point_sec  daily;
            time_point_sec  weekly;

            uint64_t primary_key()const { return 0; }
         };

         typedef eosio::multi_index< "tickers"_n, ticker > tickers;


         // asset ledger
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;


         // maintains balance of 'vested' FREEOS
         struct [[eosio::table]] vestaccount {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "vestaccounts"_n, vestaccount > vestaccounts;


         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            asset    conditional_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         // ********************************
         //using namespace eosio;

         // the registered user table
         struct [[eosio::table]] user {
           asset stake;                    // how many EOS tokens staked
           char account_type;              // user has EOS-login account, Dapp Account-login or other
           asset stake_requirement;        // the number of tokens the user is required to stake
           time_point_sec registered_time; // when the user was registered
           time_point_sec staked_time;     // when the user staked their tokens (staked tokens have a time lock)

           uint64_t primary_key() const {return stake.symbol.code().raw();}
         };

         using user_index = eosio::multi_index<"users"_n, user>;

         // the record counter table
         struct [[eosio::table]] count {
           uint32_t  count;
           uint32_t  claimevents;
           uint32_t  unvestweek;
           float     unvestpercent;
         } ct;

         using user_singleton = eosio::singleton<"usercount"_n, count>;


         // the unregistered user stake requirement
         struct [[eosio::table]] unregstake {
           asset    default_stake;

           uint64_t primary_key() const { return 0; } // return a constant (0 in this case) to ensure a single-row table
         };

         using stake_index = eosio::multi_index<"stake"_n, unregstake>;


         // FREEOS USD-price - code: freeosconfig, scope: freeosconfig
         struct [[eosio::table]] price {
           double    currentprice;
           double    targetprice;

           uint64_t primary_key() const { return 0; } // return a constant (0 in this case) to ensure a single-row table
         };

         using exchange_index = eosio::multi_index<"exchangerate"_n, price>;

         // ********************************

         // parameter table

         struct [[eosio::table]] parameter {
           name virtualtable;
           name paramname;
           std::string value;

           uint64_t primary_key() const { return paramname.value;}
           uint64_t get_secondary_1() const {return virtualtable.value;}
         };

         using parameter_index = eosio::multi_index<"parameters"_n, parameter,
         indexed_by<"virtualtable"_n, const_mem_fun<parameter, uint64_t, &parameter::get_secondary_1>>
         >;


         // stake requirement table

         struct [[eosio::table]] stakerequire {
           uint64_t    threshold;
           uint32_t    requirement_e;
           uint32_t    requirement_d;
           uint32_t    requirement_v;
           uint32_t    requirement_x;

           uint64_t primary_key() const { return threshold;}
         };

         using stakereq_index = eosio::multi_index<"stakereqs"_n, stakerequire>;


         // freeos airclaim week calendar

         struct [[eosio::table]] week {
           uint64_t    week_number;
           uint32_t    start;
           std::string start_date;
           uint32_t    end;
           std::string end_date;
           uint16_t    claim_amount;
           uint16_t    tokens_required;

           uint64_t primary_key() const { return week_number; }
         };

         using week_index = eosio::multi_index<"weeks"_n, week>;


         // claim history table - scoped on user account name
         struct [[eosio::table]] claimevent {
           uint64_t   week_number;
           uint32_t   claim_time;

           uint64_t primary_key() const { return week_number; }
         };

         using claim_index = eosio::multi_index<"claims"_n, claimevent>;

         // ********************************

         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );

         void sub_stake( const name& owner, const asset& value );
         void add_stake( const name& owner, const asset& value, const name& ram_payer );

         registration_status register_user(const name& user, const std::string account_type);

         bool checkmasterswitch();
         uint32_t getthreshold(uint32_t numusers, std::string account_type);
         week getclaimweek();
         bool eligible_to_claim(const name& claimant, week this_week);
         uint32_t updateclaimeventcount();
         uint16_t getfreedaomultiplier(uint32_t claimevents);
         void store_unregistered_stake(asset next_user_stake_requirement);
         float get_vested_proportion();

   };
   /** @}*/ // end of @defgroup freeos freeos contract

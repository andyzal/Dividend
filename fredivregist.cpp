// Last Edited: 11 Nov. by A.Zaliwski License: MIT.  The code is still in progress - may contain errors.
#include <fredivregist.hpp>

// Create whitelist
    ACTION fredivregist::allowance( uint8_t key, name vipname ){  // key = 1 proposer key=2 voter key=3 second voter
      require_auth( get_self() );

      whitelist_index white_list(get_self(), get_self().value);

      auto iterator = white_list.find(vipname.value);
          if( iterator == white_list.end() )
          {
            white_list.emplace(vipname, [&]( auto& row )
            {
              row.idno = key;
              row.user = vipname;
              row.vote = 0;
            });
          }
          else {
            white_list.modify(iterator, vipname, [&]( auto& row ) {
              row.idno = key;
              row.user = vipname;
              row.vote = 0;
             });
          }
    }

      /*---------------------
       +  create new proposal
       +---------------------
              +
              +  Proposal table is filled-up by the data coming from the frontend.
              +      - only valid proposer can create the proposal.
              */

      ACTION fredivregist::proposalnew(
        // Proposer entered parameters
        const name proposer,
        const name eosaccount,                    //!< freeos account used to receive dividends and for identification
        const char policy_name,                   //!< policy_name - (a)WayFarer or (b)WayFinder
        const char user_type,                     //!< user_type -(f)ounder or (i)nvestor
        const uint64_t usd_investment,
        const uint32_t target_freeos_price,       //TODO verify types/  const questionable?
        const uint8_t roi_target_cap,
        const uint8_t weekly_percentage,
        const bool locked,                        //!< lock dividends for selected new members. Note: When unlock cannot be locked again.
        const uint64_t rates_left,                // number of payments left under this policy (apply only to specific variants
      ){
        require_auth(proposer);
        // Is proposer white-listed for this action?
        whitelist_index white_list(get_self(), get_self().value);
        check(white_list.find(proposer.value) == white_list.end(), " :( VIP_list: You have no rights to perform this operation!!");
        //check(expires_at > current_time_point_sec(), "expires_at must be a value in the future.");
        // Assume blank proposal_struct
        const uint64_t key = 1;  //This was made assumming that will be more than one proposal (eventually use singleton)
        proposal_table proposals(get_self(), get_self().value);
        check(proposals.find(key) == proposals.end(), " :( A Proposal already exists");
        auto p = proposals.find(key);
        if ( p == proposals.end() ){
          //create new proposal
          proposals.emplace(_self, [&](auto &p) {
                      p.key = 1; // Always one, if no more than one proposal at once allowed.
                      p.eosaccount          = eosaccount;
                      p.policy_name         = policy_name;
                      p.user_type           = user_type;
                      p.usd_investment      = usd_investment;
                      p.target_freeos_price = target_freeos_price;
                      p.roi_target_cap      = roi_target_cap;
                      p.weekly_percentage   = weekly_percentage;
                      p.locked              = locked;
                      p.expires_at = current_time_point_sec() + EXPIRATION_PERIOD;
           });
           // Erase message table to make place for current errors.
           clearfront();
        } //end of if
        // Notify voters using frontend when this proposal is submitted.
      } // --- end of proposal_create ---

      /*
       +-------------------
       +  proposalclr - destroy proposal
       +-------------------
                +
                +  Destroy proposal on proposer request. Recycle RAM.
                +  Note: This not removes message messages_table
                */
ACTION fredivregist::proposalclr( name proposer) {
    require_auth(proposer);
    // is proposer on white-list
    whitelist_index white_list(_self, _self.value);
    auto v = white_list.find(proposer.value);
    if ( v == white_list.end() ){
        eosio::printf("(VIP_list): You have no rights for this operation!!");
        notify_front( 1 );
        return;
    } else { //proposer accepted
        proposaldestroy();
      };
} //end of.

      /*
      +-------------------
      +  proposalvote - voting for proposal, finalize or reject, mint NFT.
      +-------------------
              +
              +  Voting for the proposal. If both votes are positive, the proposal is written
              +  to the register. Otherwise the proposal will be destroyed.
              +
              */

  ACTION fredivregist::proposalvote(  name    voter,
                                      uint8_t vote   )
      {
        require_auth(voter);
        //voter allowed?
        whitelist_index white_list(_self, _self.value);
        auto v = white_list.find(voter.value);
        if ( v == white_list.end() ){ //wrong voter
            printf("Error 1 (VIP_list): You do not have rights for this operation!!");
            notify_front( 1 );
            return;
        }

        //voter is allowed
        check(v->key!=1, "You are proposer not voter!");
        //proposal expired?
        proposal_table proposals(get_self(), get_self().value);
        auto rec_itr = proposals.begin();
        check(rec_itr->expires_at > current_time_point_sec(), "proposal already expired.");  //Note function TODO

        //is it your first vote??
        check(v->vote==0, "This is your second vote!"); //assume: return if false (?)

        //proposal is not expired and voter is allowed - then update vote
        white_list.modify(v,_self,[&](auto& p){  p.vote = vote; }

        //verify proposal approval or refusal
        //
        const uint8_t a, b;

        for( auto iter=white_list.begin(); iter!=white_list.end; iter++ ) {
          // ignore proposer iter->idno == 1
          if( iter->idno == 2 ){ //take first voter result
            a = iter->vote;
          }
          if( iter->idno == 3 ){ //take second voter result
            b = iter->vote;
          }
        } // alternative solution for the above is to use get_index()

        if ( (a==1)||(b==1) ) { //proposal refused - destroy it :(
          proposaldestroy();
        return;
        }

        if ( (a==2)&&(b==2) ) { //proposal accepted - finalize it :)
        //Copy proposal to the investors register (this is equal to minting NFT)
        rec_itr = proposal.begin();
        register_table register(_self, _self.value);
        auto reg_itr = register.end();
        register.emplace(_self, [&]( auto& r ){
          r.eosaccount              = rec_itr->eosaccount;
          r.policy_name             = rec_itr->policy_name;
          r.user_type               = rec_itr->user_type;
          r.usd_investment          = rec_itr->usd_investment;
          r.target_freeos_price     = rec_itr->target_freeos_price;
          r.roi_target_cap          = rec_itr->roi_target_cap;
          r.weekly_percentage       = rec_itr->weekly_percentage;
          //Note: This register record is single NFT by itsel
          r.mint_date               = current_time_point_sec();
          r.locked                  = rec_itr->locked;
          r.rates_left              = rec_itr->rates_left;
          //sharefraction - this is computed field (later) TODO discuss                                                     ;
        });
        eosio::printf("Proposal accepted and saved. NFT minted.");
        notify_front( 5 );
        proposaldestroy();
        }

      } //end.



      /* this is DIVIDEND FRAME
      +-------------------
      +  dividend compute
      +-------------------
              +
              +  Counting and write computing code (based on prescribed policy) to each investor record.
              +  The code is used in the next step to count the dividend.
              +
              */

  ACTION fredivregist::dividcomputed( name dividend_acct ){
    require_auth(_self);

  } //end.

      /* this is DIVIDEND FRAME
      +-------------------
      +  dividend delivery
      +-------------------
              +
              +  The action transfer eligible amount of tokens from pre-designed account to account of each investor
              +  according to pre-computed earlier code.
              +
              */

  ACTION fredivregist::dividenddeliv( name dividend_acct ){
    require_auth(_self);

    //TODO
    register_table register(_self, _self.value);
    for (auto iter = register.begin(); iter != register.end(); iter++)
    {
           // handle record
           // ...
    }
  } //end.

  // Note: the above may not work due to 150ms transaction limitation. In that case altternative solution
  // is more complicated:  Processing table page by page with keeping the lower and upper bound each time
  // register_table register(_self, _self.value);
  // auto idx = register.get_index<"byname"_n>();
  // auto itr_start = idx.lower_bound(name("pending").value);
  // auto itr_end = idx.upper_bound(name("pending").value);
  // for (itr_start != itr_end; itr_start++)
  //   {
  //      // handle item record (they should all be of state "pending")
  //   }

  ACTION fredivregist::dividendchown( name owner, name new_owner ){ // Change ownership
    //verify existence of owner account
    require_auth( owner );
    //verify existence of target (new owner) account
    // Search register for owner - exit if not found



    //make Change


    //Notify whatever
  } //end.

      //                        //
      // Small Helper Functions //
      //                        //

// List of errors to export to the frontend for interpretation
void fredivregist::notify_front( uint8_t number ){
  messages_table errortable(_self, _self.value);
  auto e = errortable.end();
  errortable.emplace(_self, [&](auto &e) {
    e.key = e.key + 1; //verify TODO
    e.errorno = number;
  }
} //end of.

// Clear list of errors when start new proposal - no conditions verified
void fredivregist::clearfront( void ){
  messages_table errortable(_self, _self.value);
  auto e = errortable.begin();
  while (e != errortable.end()) {
      e = errortable.erase(rec_itr);
  }
} //end of.

// Clear/destroy proposal - Warning : this is only for internal use as it is unconditional.
void fredivregist::proposaldestroy( void ){
    proposal_table proposals(get_self(), get_self().value);
    // Delete proposal - release RAM
    auto rec_itr = proposals.begin();
    while (rec_itr != proposals.end()) {
      rec_itr = proposals.erase(rec_itr);
    }
}

EOSIO_DISPATCH(fredivregist, (allowance)(proposalnew)(proposalclr)
                             (proposalvote)(dividcomputed)(dividdelivery) )

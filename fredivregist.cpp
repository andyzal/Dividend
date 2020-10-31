#include <fredivregist.hpp>

// Create whitelist
    ACTION fredivregist::allowance( uint64_t key, name vipname ){
      require_auth( get_self() );  

      whitelist_index white_list(get_self(), get_self().value);

      auto iterator = white_list.find(vipname.value);
          if( iterator == white_list.end() )
          {
            white_list.emplace(vipname, [&]( auto& row ) 
            {
              row.key = key;
              row.vip_user = vipname;
              row.vote = 0;
            });
          }
          else {
            white_list.modify(iterator, vipname, [&]( auto& row ) {
              row.key = key;
              row.vip_user = vipname;
              row.vote = 0;
             });
          }   
    }



      /*-------------------
       +  proposal_create
       +-------------------
              +
              +  Proposal table is filled-up by the data coming from the frontend.
              +      - only valid proposer can create the proposal.
              */

      ACTION fredivregist::proposalnew(
        // Proposer entered parameters
        const name proposer,
        const name vaccount,                      //!< DAPP account (if any)
        const name eosaccount,                    //!< freeos account used to receive dividends and for identification
        const char policy_name,                   //!< policy_name - (a)WayFarer or (b)WayFinder
        const char user_type,                     //!< user_type -(f)ounder or (i)nvestor
        const uint64_t usd_investment,
        const uint32_t target_freeos_price,       //TODO verify types/  const questionable?
        const uint8_t roi_target_cap,
        const uint8_t weekly_percentage,
        const bool locked,                        //!< lock dividends for selected new members. Note: When unlock cannot be locked again.
        const uint8_t approvals,                  // voters approvals - must be three (3) exactly! to proceed.
        const uint64_t rates_left,                // number of payments left under this policy (apply only to specific variants
        const time_point_sec& expires_at
      ){
        require_auth(proposer);
        // Is proposer white-listed for this action?
        whitelist_index white_list(get_self(), get_self().value);
        check(white_list.find(proposer.value) == white_list.end(), " :( VIP_list: You have no rights to perform this operation!!");
        check(expires_at > current_time_point_sec(), "expires_at must be a value in the future.");
        // Assume blank proposal_struct
        const uint64_t key = 1; 
        proposal_table proposals(get_self(), get_self().value);
        check(proposals.find(key) == proposals.end(), " :( A Proposal already exists - it should be cleared out!");
        auto p = proposals.find(key);
        if ( p == proposals.end() ){
          // No proposal so far - create new
          proposals.emplace(_self, [&](auto &p) {
                      p.key = 1; // Always one, if no more than one proposal at once allowed.
                      p.vaccount = vaccount;
                      p.eosaccount = eosaccount;
                      p.policy_name = policy_name;
                      p.user_type = user_type;
                      p.usd_investment = usd_investment;
                      p.target_freeos_price = target_freeos_price;
                      p.roi_target_cap = roi_target_cap;
                      p.weekly_percentage = weekly_percentage;
                      p.locked = locked;
                      p.approvals = 0; // no approvals yet
                      p.created_at = current_time_point_sec();
                      p.expires_at = expires_at;

           });
        } //end of if
        // Notify voters using frontend when this proposal is submitted.
      } // --- end of proposal_create ---


      /*-------------------
       +  proposal_clear
       +-------------------
                +
                +  Recycle RAM after proposal not longer needed.
                +     
                */

      ACTION fredivregist::proposalclr() {
        require_auth(get_self());

        proposal_table proposals(get_self(), get_self().value);
        // Delete all records in _proposal table
        auto rec_itr = proposals.begin();
        while (rec_itr != proposals.end()) {
          rec_itr = proposals.erase(rec_itr);
          }
        }

// enforced earlier expiration of a proposal
ACTION fredivregist::expire(const name proposal_name) { //TODO change to proposer 
    proposals proposal_table(_self, _self.value);
    auto itr = proposal_table.find(proposal_name.value);

    check(itr != proposal_table.end(), "proposal not found.");
    check(!itr->is_expired(), "proposal is already expired.");

    auto proposer = itr->proposer; //TODO this cannot be taken from proposal but from allowances table
    require_auth(proposer);

    proposal_table.modify(itr, proposer, [&](auto& row) {
        row.expires_at = current_time_point_sec();
    });
}


      /*
      +-------------------
      +  proposal_vote
      +-------------------
              +
              +  Voting for the proposal. If this is last vote and both votes are positive, the proposal is written
              +  to the register. Otherwise the proposal will be destroyed.
              +
              */

      // Only pre-defined voters can vote for the proposal. 
      // TODO Exclude the same voter voting twice.


      ACTION fredivregist::proposalvote(
                                          name voter,                      //
                                          uint8_t vote
                                          )
      {
        
        require_auth(voter);
        // Also verify is voter whitetlisted for this action?
        whitelist_index white_list(_self, _self.value);
        check(white_list.find(voter.value) == white_list.end(), " :( VIP_list: You have no rights for this operation!!");

//Whitelist will keep last voter name.    " :( VIP_list: You voted already for this proposal. This vote will be ignored!"); 

        check(!row.is_expired(), "cannot vote on an expired proposal.");
// Are you the latest voter ??? :)
        // calculate the user number when registered
        
        
        
        //First voter is different than second??
        if ( last_vote() == true ) {
           if ( proposal_accepted() ) {
                copy_proposal_to_register(); // include minting
                notice_all('accepted');  //May be not possible to do - TODO add verify result ACTION
            } else { // proposal not accepted
            notice_all('rejected');
            };
            proposal_remove();
        } else  // not the last voter
            update_results();
            lock_current_voter();
        } // end of proposal vote.

        //uint64_t mint_date;                 // current date (when this register record was created)
        // When copy proposal to register also clean up the vote indicator column for whitelist to zeros. One if someone was voting. 

//How to make notification if this operattion is finished??

// proposer can have query on proposal voting and finalization results. The status of this operation should be written somewhere? Where?

///ACTION // Change ownership 

//vote and finalize proposal in one actions: If two votes positive mint NFT and create the new register entry according to proposal content.
///Action Dividend (go through the register and divide a whole account content - make the transfers. Look at Cmichael at the "consider". 
//the proposer may query the systems for the message log of the latest operation performed (only for the last proposal). 
///Tasks to do
///- Write the frame code containing logic to incorporate the two following actions:
///- Calculate and implement the formula translating the information from investor/founder NFT into percentage of dividend 
///from current profit account.
///- Create inline transfer action from profit account into each NFT account from the register. (transfers amount calculated 
// in previous step to the investor/founder - this action will be repaeted many times at once. Note time for transaction limits.
///- Write code reporting the results of current dividend round to everybody interested and creating the log file.
////- Write demo for potential investors to simulate the consequences of different policy strategy selection in relationships 
///  to the freeos profits (Vue only).
///- create action allowing NFT ownership transfer (gift or inheritance but not selling) (Also possible to transfer ownership
///to John Doe to cancel the NFT).
///- create NFT sell action (optional) only using Freeos tokens. 










EOSIO_DISPATCH(fredivregist, (allowance)(proposalnew)(proposalclr))

#include "freeosdivide.hpp" 
#include <math.h>

// After finishing the tests this action may be called only by multisig. 
ACTION freeosdivide::allowance( uint8_t code, name vipname )
{  // key = 1 proposer key=2 voter key=3 second voter.
   check( (code==1)||(code==2)||(code==3), "Wrong code!");  
   require_auth( get_self() );
   // It should be exactly three records. Codes cannot repeat.
   // New record added always at the end.
   whitelist_index white_list(get_self(), get_self().value);
   uint8_t i=0;
   for( auto iter=white_list.begin(); iter!=white_list.end(); iter++ ){
      check(iter->idno!=code, "Repeated eligibility code!");
      i++; // Count the records
   }
   if(i<3){
     auto iterator = white_list.end();  
     white_list.emplace(get_self(), [&]( auto& row )
       {
        row.idno = code;
        row.user = vipname;
        row.vote = 0;
       });
   }
}

// After finishing the tests this action may be called only by multisig. 
ACTION freeosdivide::removeallow(){
  require_auth( get_self() );
  whitelist_index white_list(get_self(), get_self().value);
  // Delete all records in _messages table
  auto itr = white_list.begin();
    while (itr != white_list.end()) {
       itr = white_list.erase(itr);
    }
}

ACTION freeosdivide::proposalnew(
  		  const name      proposer,
        const name      eosaccount,          //!< freeos account used to receive dividends and for identification
        const uint8_t   roi_target_cap,      // 1-(I)teration, 2-(H)orizontal, 3-(V)ertical
        const double    weekly_percentage,
        const asset     threshold,           // max value for total (cap=2) or weekly (cap=3) pay-cut actions
        const uint32_t  rates_left,          // number of dividend rates which cause pay-stop (roi_target_cap=1)
        const bool      locked               //!< lock dividends for selected new founders. Note: When unlock cannot be locked again.
    )
{     

	//Verify proposer against white_list but not against other authorization.
  require_auth(proposer);
	// "El Chapo" version of a singleton :)


// The proposer should be found and it should be first on the list. 
whitelist_index white_list(get_self(), get_self().value);
auto v = white_list.find(proposer.value);  // Is the proposer on the list?
check( (v!=white_list.end()),                 "No proposer on the list?!"); 
check( (v->idno==1),                          "On the list, but is not the proposer!"); 
check( is_account( eosaccount ), "eosaccount does not exist"); 

if (roi_target_cap==1) check( (rates_left>0), "Iteration Cap, but rates_left not above zero!");
if (roi_target_cap!=1) { 
    check( threshold.is_valid(), "invalid threshold" );
    check( threshold.amount > 0, "must issue positive threshold" );
}

proposal_table proposals(get_self(), get_self().value);
  auto pro_itr = proposals.find(1);
  if( pro_itr == proposals.end() )
 	{
  	 proposals.emplace(get_self(), [&](auto &p) { 
            p.key                 = 1;
            p.eosaccount          = eosaccount; 
            p.roi_target_cap      = roi_target_cap;
            p.weekly_percentage   = weekly_percentage;
            p.threshold           = threshold;
            p.rates_left          = rates_left;
            p.locked              = locked;
            p.accrued             = asset(0,symbol("FREEOS",4) ); 
            p.expires_at          = now()+EXPIRATION_PERIOD;
           });
	}
  else
 	{
  	proposals.modify(pro_itr, get_self(), [&](auto &p) {
            p.key                 = 1;
            p.eosaccount          = eosaccount; 
            p.roi_target_cap      = roi_target_cap;
            p.weekly_percentage   = weekly_percentage;
            p.threshold           = threshold;
            p.rates_left          = rates_left; 
            p.locked              = locked;
            p.accrued             = asset(0,symbol("FREEOS",4) ); 
            p.expires_at          = now()+EXPIRATION_PERIOD;
           });
  	}    	
} //end of action.


ACTION freeosdivide::proposalvote(  const name voter,
                                    const uint8_t vote )
{
        require_auth(voter);
        check( (vote>0)&&(vote<3), "Vote value out of range!"); 

        //voter allowed?
        freeosdivide::whitelist_index white_list(get_self(), get_self().value);
        auto v = white_list.find(voter.value); // Is the voter on the list?
        check( (v!=white_list.end()), "Wrong voter!");
        check( (v->idno!=1), "Not a voter!");
        check( (v->vote==0), "Second vote not allowed!"); // diffrent than zero if already voted 

        //voter is allowed
        proposal_table proposals(get_self(), get_self().value);
        auto rec_itr = proposals.begin();
        check(rec_itr->expires_at >= now(), "Proposal already expired.");  

        //proposal not expired and voter allowed - update vote
        white_list.modify(v,get_self(),[&](auto& p){
          p.vote = vote;
        });

        /*      verify proposal approval or refusal   0?    1-no  2-yes         Voting Table
                voter a      voter b
                    0            0       - no one voted          sum=0     a*b == 0   voting not finished
                    1            0       - only one voted        sum=1     a*b == 0
                    0            1       - only one voted        sum=1     a*b == 0

                    1            1       - both refused          sum=2     a*b != 0   finished and refused
                    2            1       - one refused           sum=3     a*b != 0
                    1            2       - one refused           sum=3     a*b != 0

                    2            2       - both accepted         sum=4     a*b != 0 and a+b==4 finished and accepted */

        uint8_t a, b;
        
        auto idx = white_list.get_index<"byidno"_n>();
        auto iter = idx.find(2);
        //eosio_assert(iter == white_list.end(), "Wrong whitelist_index 2."
        a = iter->vote; 
        iter = idx.find(3);
        //eosio_assert(iter == white_list.end(), "Wrong whitelist_index 3."
        b = iter->vote;
        
        a = a * b; //See the Voting Table above.
  
        if(a!=0)
        { 
          if (a==4)   
            { // proposal accepted - finalize it :)
              // Copy proposal to the investors/founder register (this is NFT minting)
              auto rec_itr = proposals.begin();
              register_table registers( get_self(), get_self().value );
              auto reg_itr = registers.end(); //adding to register at the end
              registers.emplace(get_self(), [&]( auto& r ){
              r.nft_id                  = registers.available_primary_key(); //autoincrement
              r.eosaccount              = rec_itr->eosaccount; //NOTE: This is secondary key!
              r.roi_target_cap          = rec_itr->roi_target_cap;
              r.weekly_percentage       = rec_itr->weekly_percentage;
              r.mint_date               = time_point_sec(current_time_point().sec_since_epoch());
              r.locked                  = rec_itr->locked;
              r.accrued                 = rec_itr->accrued;
              r.threshold               = rec_itr->threshold;
              r.rates_left              = rec_itr->rates_left;    
              });
              printf("Proposal accepted and saved. NFT minted.");
            }

            //erase content of the proposal 
            auto pro_itr = proposals.find(1);        
            proposals.modify(pro_itr, get_self(), [&](auto &p) 
            {
              p.eosaccount          = "erased"_n; 
              p.roi_target_cap      = 0;
              p.weekly_percentage   = 0.0;
              p.threshold           = asset(0,symbol("FREEOS",4) ); 
              p.rates_left          = 0;
              p.locked              = true;
              p.accrued             = asset(0,symbol("FREEOS",4) ); 
              p.expires_at          = now();
            });
            // erase voting results 
            for( auto iter=white_list.begin(); iter!=white_list.end(); iter++ )
            {
              white_list.modify(iter, get_self(), [&]( auto& row )
              {
                row.vote = 0;
              }); 
            }
        } // a==4   
        // if a==0 do nothing - voting not finished
} //end.
//
//======================================================================

/* --------------------------------------------------------------------------------------------------------
 * this is the DIVIDEND FRAME 
 */


     /*
      +-----------------------------------
      +  dividcomputed - dividend computed
      +-----------------------------------
              +
              +  Count weekly_percentage from all the NFT's belongings to a given person.              
              +  Summary table keeps NFT data summarized for each person.   
              +
              */
  ACTION freeosdivide::dividcomputed() 
  {
    require_auth(_self);
    register_table registers( get_self(), get_self().value );
    total_index    summary(   get_self(), get_self().value );

    // Read profit balance                                                         
    asset profit = asset(0,symbol("FREEOS",4) ); 
    accounts accountstable(tokencontra, get_self().value );              //inline!   Token contract   (replaces "eosio.token"_n)
    const auto& ac=accountstable.get( symbol_code("FREEOS").raw() );
    profit = ac.balance; 
    print("profit_",profit); 

    /// symbol_code freeos = symbol_code("FREEOS");
    /// auto user_freeos_account = user_accounts.find(freeos.raw());

    // summary - Summarize weekly_percentage from all user's NFT's in one record for each user.
    // Only for eligible users (verify: accrued, threshold, etc.). 
 
    // Review the NFT register sequentially 
    for (auto iter = registers.begin(); iter != registers.end(); iter++)
    {       // extract working variables
            double   weekly_percentage = iter->weekly_percentage;
      const name     user              = iter->eosaccount;
      const bool     locked            = iter->locked;
            asset    accrued           = iter->accrued;
      const uint8_t  cap_type          = iter->roi_target_cap;
            asset    threshold         = iter->threshold;
            uint32_t ratesleft         = iter->rates_left; 

      auto idx = summary.find(user.value); 

      // Count how much to pay for this NFT in FREEOS tokens:
      asset to_receive = asset(0,symbol("FREEOS",4) ); 
      to_receive.amount = ((profit.amount/100)*weekly_percentage);   

      switch( cap_type ) 
      {
        case 1:   //WayFarer/investor - Iteration Cap
        {
            if (ratesleft>0)
            { // still paid 
              //print("case 1 inside...");
              //Add to_receive to summary for this user.   
              if( idx == summary.end() )
              {
                summary.emplace( get_self(), [&]( auto& row )
                {
                  row.user = user;
                  row.to_receive = asset(0,symbol("FREEOS",4) ); //preformatting the table 
                  row.to_receive.amount = to_receive.amount;
                });
              }
              else 
              {
                summary.modify(idx, get_self(), [&]( auto& row ) {
                row.to_receive.amount += to_receive.amount;
                });
              };
              //update rates_left in register 
              registers.modify(iter,_self,[&](auto& p){ 
                p.rates_left = ratesleft - 1;  
              });
            }
            else print("case 1 - no more rates left");
            break;
         } 
          
         case 2:   //WayFarer/investor - Horizontal Cap
         {
              print("accrued",accrued.amount, "threshold", threshold.amount);
              
              if( accrued.amount<=threshold.amount) 
              { //stil paid
                       
                // update accrued and eventually correct payment if last (reduce to_receive) 
                asset overpayment = asset(0,symbol("FREEOS",4) );   
                asset simulated   = asset(0,symbol("FREEOS",4) );
                //simulated.amount  = (accrued.amount + to_receive.amount)/10000;
                simulated  = accrued + to_receive;

                // always last rate will have more or less overpayment - cut it here
                if ( simulated.amount>=threshold.amount )

                { // this is last rate to pay for this NFT.
                  // How much simulated is bigger than threshold?
                  //overpayment.amount = simulated.amount - threshold.amount; 
                  overpayment = simulated - threshold; 
                  //to_receive.amount = to_receive.amount - overpayment.amount;  
                  to_receive = to_receive - overpayment;                    
                  print("over=", overpayment, "_toreceive=_", to_receive);
                }   

                //Add 'to_receive' to 'summary' for this user.  
                if( idx == summary.end() )
                {
                  summary.emplace( get_self(), [&]( auto& row ){
                   row.user = user;
                   row.to_receive = asset(0,symbol("FREEOS",4) ); //preformatting the table 
                   row.to_receive = to_receive;
                  });
                }
                else 
                {
                  summary.modify(idx, get_self(), [&]( auto& row ) {
                    row.to_receive.amount += to_receive.amount;
                  });
                  
                }
                registers.modify(iter,_self,[&](auto& p){
                    p.accrued = accrued + to_receive;
                });      
              } 
              break;
             
          }  

          case 3:   //WayFinder/Founder - Vertical Cap 
            {  
              if (!locked)
              {
                //print("case 3 inside ...");
                  if(to_receive.amount>=threshold.amount) 
                  {
                    to_receive = threshold;                   
                  }

                //Add to_receive to summary for this user.   
                if( idx == summary.end() )
                {
                  summary.emplace( get_self(), [&]( auto& row ){
                   row.user = user;
                   row.to_receive = asset(0,symbol("FREEOS",4) ); //preformatting the table 
                   row.to_receive = to_receive;
                  });
                }
                else 
                { 
                  summary.modify(idx, get_self(), [&]( auto& row ) {  
                    row.to_receive.amount += to_receive.amount;
                   }); 
                } 
              print("_3to_receive:_", to_receive);      
              }  
              break;
            } //end of case 3..
        } //end of switch.     
      }    //end of for.  
    }
 
    /*
     +---------------------------------
     +  regtransfer - dividend transfer
     +---------------------------------
             +
             +  Take summary table and according to its entry transfer proportional dividend to each user account.
             +  Leftover is transferred to FreeDAO account.              
             +  At the end the summary table must be erased. 
             +
             */
    ACTION freeosdivide::regtransfer()    //This header should be removed in final version to make one action with the above action.
    {  
      // Just do the transfer to all eligible investors and founders.
      require_auth(_self);                                                      //to remove later 
      total_index summary( get_self(), get_self().value );
      for (auto idx = summary.begin(); idx != summary.end(); idx++)
      {

        name user = idx->user;
        asset quantity = asset(0,symbol("FREEOS",4) ); 
        quantity = idx->to_receive; 
        
        if(quantity.amount>0){
          action dtransfer = action(
            permission_level{get_self(),"active"_n},
            name(freeos_acct),   
            "transfer"_n,
            std::make_tuple(get_self(), user, quantity, std::string("yours weekly dividend"))
          );
          dtransfer.send();
        }
      }
      
      //All user's transfers done. Time to clear up the temporary table 'summary'.
      auto rc_itr =  summary.begin();
      while (rc_itr != summary.end()) 
      {
         rc_itr =  summary.erase(rc_itr);
      }

      //Submit leftower tokens from freeosdivide to freeosdaodao: 
      //read final (profit) balance after dividends payment:                                                          
      asset profit   = asset(0,symbol("FREEOS",4) );  
      accounts accountstable(tokencontra, get_self().value );
      const auto& ac=accountstable.get( symbol_code("FREEOS").raw() );
      profit = ac.balance; 

      asset quantity = asset(50000,symbol("FREEOS",4) );  //Keep 5 tokens for eventual CPU. RAM payments ??
      print("profit is _____", profit.amount); 
 
      asset to_receive = asset(profit.amount, symbol("FREEOS",4) ); 
      to_receive = to_receive - quantity;
  
      print("**profit is _____", to_receive);
            
      action stransfer = action(
             permission_level{get_self(),"active"_n},
             name(freeos_acct),                                                                                              
             "transfer"_n,
             std::make_tuple(get_self(), daoaccount, to_receive, std::string("yours weekly leftover"))
      );
      stransfer.send();  
      //      }  
   }  //end of regtransfer.
//-------------------------------------------------------------------------------------

ACTION freeosdivide::zerofortest()   
{   
    //used for TESTS only - remove later. This is to clean up the results of previous tests.
    require_auth(get_self());

    register_table registers( get_self(), get_self().value );
    for( auto ite=registers.begin(); ite!=registers.end(); ite++ ){
         registers.modify(ite, get_self(), [&]( auto& row1 )
         {
           row1.accrued = asset(0,symbol("FREEOS",4) );
         }); 
    }  

    total_index summary( get_self(), get_self().value );
    for( auto iter=summary.begin(); iter!=summary.end(); iter++ ){
         summary.modify(iter, get_self(), [&]( auto& row )
         {
           row.to_receive = asset(0,symbol("FREEOS",4) ); //preformatting the table 
         }); 
    }  
} // end of test 

//-----------------------------------------------------------------------------------------------------------------
//Clean up mess after tests - need to have variable scope :)
ACTION freeosdivide::clear(name user) 
{
 require_auth(get_self());
  whitelist_index white_list(get_self(), get_self().value);
   uint8_t i=0;
   for( auto iter=white_list.begin(); iter!=white_list.end(); iter++ ){
    white_list.modify(iter, get_self(), [&]( auto& row )
       {
        row.vote = 0;
       }); 
  }  
} //end of clear.

// Reset the vote results. ONLY for test.   //TODO user is not used. 
ACTION freeosdivide::votesreset11(name user){
  require_auth(get_self());
  whitelist_index white_list(get_self(), get_self().value);
   uint8_t i=0;
   for( auto iter=white_list.begin(); iter!=white_list.end(); iter++ ){
    white_list.modify(iter, get_self(), [&]( auto& row )
       {
        row.vote = 0;
       }); 
   }
}

// List of errors to export to the frontend for interpretation - internal only!
void freeosdivide::notify_front( uint8_t number ) 
{
  messages_table errortable( get_self(), get_self().value );
  auto e = errortable.end();
  errortable.emplace( get_self(), [&](auto &e) {
    e.key = e.key + 1; //verify TODO
    e.errorno = number;
  } );
} //end of.

// Clear list of errors when start new proposal - no conditions verified - internal only!
void freeosdivide::clearfront() {
    messages_table  errortable(get_self(), get_self().value);
    auto rec_itr =  errortable.begin();
  while (rec_itr != errortable.end()) {
         rec_itr =  errortable.erase(rec_itr);
  }
} //end of.

//Only for tests.  It is not used by the contract yet. 
void freeosdivide::prop_reset() {  //erase content of proposal 
  proposal_table proposals(get_self(), get_self().value);
  require_auth(get_self());
  auto pro_itr = proposals.find(1);
  proposals.modify(pro_itr, get_self(), [&](auto &p) {
            p.eosaccount          = "erased"_n; 
            p.roi_target_cap      = 0;
            p.weekly_percentage   = 0;
            p.threshold           = asset(0,symbol("FREEOS",4) ); 
            p.locked              = true;
            p.accrued             = asset(0,symbol("FREEOS",4) ); 
            p.expires_at          = now();
           });
} //prop_reset


/* Change NFT ownership, Note this must receive a NFT key from frontend to work */ 
ACTION freeosdivide::regchown(name userfrom, name userto, uint64_t nft_id){
  check( userfrom != userto, "cannot transfer to self" );
  require_auth(userfrom);    
  check( is_account( userto ), "userto account does not exist");

  register_table registers( get_self(), get_self().value );
  auto pro_itr = registers.find(nft_id);

  registers.modify(pro_itr, get_self(), [&](auto &p) {
            p.eosaccount = userto; 
  });
}     
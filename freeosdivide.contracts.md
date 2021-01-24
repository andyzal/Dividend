<h1 class="contract">
   proposalnew
</h1>
<h2>create new proposal</h2>

**PARAMETERS:** 
* __proposer__ has right to initiate new proposal 
* __eosaccount_ the dividend target account for founder or investor
* __roi_target_cap__ Value 1,2 or3. Itis 1-(I)teration, 2-(H)orizontal, or 3 for (V)ertical cap. 
(* __expires_at__ proposal expiration time (default one hour). After that time proposal is cancelled and cannot be voted. Actually, used internally).
* __weekly_percentage__ of dividend transrerred into __eosaccount__ this week round
* __threshold__ different meaning depend on "roi_target_cap"=1 countdown counter of rounds to pay,  if 3 - the highest amount of dividend to pay this week
* __accrued__  total sum of dividends up-to-date - used only when roi_target_cap=2; if accrued>=threshold -> stop-pay 
* __locked__ temporary lock for new founders (can be used only once)

**INTENT:** The intent of [[proposalnew ]] is to create a new proposal introducing a new investor or founder to the register - When proposal is accepted the investor of founder receives NFT stored as register entry (NFT-Non Fungible Token) allowing to receive weekly pre-defined profit-based dividend.  The notification of a new proposal should be emailed to both voters by the frontend. The proposer account is priviledged (VIP) account. 

**TERM:** The proposal must be accepted by pre-defined voters not later than one hour from its issue, otherwise will be destroyed. 

<h1 class="contract">
   proposalvote
</h1>
<h2>vote on proposal</h2>

**PARAMETERS:** 
* __voter__ is accountname (must be whitelisted) of a voter for identification
* __vote__ is a value of a vote ( 0-didn't voted yet or ignored, 1-no, 2-yes). 
       
**INTENT:** The intent of proposalvote is to accept (or not) the issued proposal. If proposal will be accepted by both pre-defined priviledged voters, it will be copied to investor/founder register and NFT will be minted. From this moment investor/voter will be eligible to receive policy based share from dividends. 
Otherwise, proposal not accepted by voters, will be destroyed. 

**TERM:**  Voters have one hour (counted from proposal issue) to take the decision on proposal acceptance.

<h1 class="contract">
   proposalclr
</h1>
<h2>cancel proposal before expiration</h2>

**PARAMETERS:** 
* __proposer__ proposer account

**INTENT:** The intent of {{ proposalclr }} is to allow the proposer cancelling the pending proposal before its one hour normal expiration. 

**TERM:** The voters will be notified on earlier proposal cancellation. Note: Accepted proposal becomes new NFT in the register and not exists anymore as a proposal. 

<h1 class="contract">
   allowance
</h1>
<h2>Creates whitelist</h2>>

**PARAMETERS:** 
* __key__ this is 1- for proposer, 2- for first voter, 3-for second voter.
* __vipname__ related proposer or voter account which will be used later for identification of proposer or voter.

**INTENT:** The intent of {{ allowance }} is to create a list of priviledged proposers and voters to create and accept the further proposals.  

**TERM:** The priviledged proposers and voters table allowed to deal with proposals is created only from command line and requires multisig. 

<h1 class="contract">
   dividcomputed
</h1>
<h2>dividend computed</h2>

**PARAMETERS:** 
No parameters - Its used NFT register internally to verify dividend eligibility and to create totals for all the shares of a given investor/founder. 

**INTENT:** The intent of {{ dividcomputed }} is to pre-count all dividend eligibility for each set of NFTs belonging to a given founder/investor.  This action will be run once a week.

**TERM:**   N/A

<h1 class="contract">
   regtransfer
</h1>
Note: In production version this action code becomes part of dividcomputed action 
**PARAMETERS:** 
* __dividend_acct__ this is account keeping all the pool of received profits. For now must be identical with the account on which this contract is installed. 
* __freeos_acct__ the profit not distributed to founders/investors will be transferd to this account.

**INTENT:** The intent of regtransfer is to make pre-counted dividend transfer to the founders and investors from the NFT register with values proportional to their  NFTs owned. The remaining, not distributed part of the profit will be transfered to FreeDAO account.
Note: If there will be any special/target/purpose FreeDAO accounts to receive part of their dividend it must be registered the same way like other accounts (by creating and accepting proposal).

**TERM:** These terms and conditions are defined by the other documents.

<h1 class="contract">
   regchown
</h1>

**PARAMETERS:** 

* __owner__ current NFT owner 
* __new_owner__ new NFT owner 
* __nftid__ NFT_id must be received from the front-end

**INTENT:** The intent of regchown is to allow changes in NFT ownership (e.g. only by inheritance or gift). 

**TERM:**  TBA 

<h1 class="contract">
 gbalance
</h1>


 
  

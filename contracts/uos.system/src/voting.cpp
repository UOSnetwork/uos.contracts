#include <eosio/crypto.hpp>
#include <eosio/datastream.hpp>
#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/privileged.hpp>
#include <eosio/serialize.hpp>
#include <eosio/singleton.hpp>

#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <algorithm>
#include <cmath>

namespace eosiosystem {

   using eosio::const_mem_fun;
   using eosio::current_time_point;
   using eosio::indexed_by;
   using eosio::microseconds;
   using eosio::singleton;

   void system_contract::regproducer( const name& producer, const eosio::public_key& producer_key, const std::string& url, uint16_t location ) {
      check( url.size() < 512, "url too long" );
      check( producer_key != eosio::public_key(), "public key should not be the default value" );
      require_auth( producer );

      auto prod = _producers.find( producer.value );
      const auto ct = current_time_point();

      if ( prod != _producers.end() ) {
         _producers.modify( prod, producer, [&]( producer_info& info ){
            info.producer_key = producer_key;
            info.is_active    = true;
            info.url          = url;
            info.location     = location;
            if ( info.last_claim_time == time_point() )
               info.last_claim_time = ct;
         });

         auto prod2 = _producers2.find( producer.value );
         if ( prod2 == _producers2.end() ) {
            _producers2.emplace( producer, [&]( producer_info2& info ){
               info.owner                     = producer;
               info.last_votepay_share_update = ct;
            });
            update_total_votepay_share( ct, 0.0, prod->total_votes );
            // When introducing the producer2 table row for the first time, the producer's votes must also be accounted for in the global total_producer_votepay_share at the same time.
         }
      } else {
         _producers.emplace( producer, [&]( producer_info& info ){
            info.owner           = producer;
            info.total_votes     = 0;
            info.producer_key    = producer_key;
            info.is_active       = true;
            info.url             = url;
            info.location        = location;
            info.last_claim_time = ct;
         });
         _producers2.emplace( producer, [&]( producer_info2& info ){
            info.owner                     = producer;
            info.last_votepay_share_update = ct;
         });
      }

   }

   void system_contract::unregprod( const name& producer ) {
      require_auth( producer );

      const auto& prod = _producers.get( producer.value, "producer not found" );
      _producers.modify( prod, same_payer, [&]( producer_info& info ){
         info.deactivate();
      });
   }

   void system_contract::regcalc(const name calculator, const std::string &url, uint16_t location) {
       check( url.size() < 512, "url too long" );
       require_auth( calculator );

       auto calc = _calculators.find( calculator.value );

       if ( calc != _calculators.end() ) {
           _calculators.modify( calc, calculator, [&]( calculator_info& info ){
               info.is_active    = true;
               info.url          = url;
               info.location     = location;
           });
       } else {
           _calculators.emplace( calculator, [&]( calculator_info& info ){
               info.owner         = calculator;
               info.total_votes   = 0;
               info.is_active     = true;
               info.url           = url;
               info.location      = location;
           });
       }
   }

   void system_contract::unregcalc(const name calculator) {
       require_auth( calculator );

       const auto& calc = _calculators.get( calculator.value, "calculator not found" );

       _calculators.modify( calc, same_payer, [&]( calculator_info& info ){
           info.deactivate();
       });
   }

   void system_contract::update_elected_producers( const block_timestamp& block_time ) {
      _gstate.last_producer_schedule_update = block_time;

      auto idx = _producers.get_index<"prototalvote"_n>();

      std::vector< std::pair<eosio::producer_key,uint16_t> > top_producers;
      top_producers.reserve(21);

      for ( auto it = idx.cbegin(); it != idx.cend() && top_producers.size() < 21 && 0 < it->total_votes && it->active(); ++it ) {
         top_producers.emplace_back( std::pair<eosio::producer_key,uint16_t>({{it->owner, it->producer_key}, it->location}) );
      }

      if ( top_producers.size() == 0 || top_producers.size() < _gstate.last_producer_schedule_size ) {
         return;
      }

      /// sort by producer name
      std::sort( top_producers.begin(), top_producers.end() );

      std::vector<eosio::producer_key> producers;

      producers.reserve(top_producers.size());
      for( const auto& item : top_producers )
         producers.push_back(item.first);

      if( set_proposed_producers( producers ) >= 0 ) {
         _gstate.last_producer_schedule_size = static_cast<decltype(_gstate.last_producer_schedule_size)>( top_producers.size() );
      }
   }

   double stake2vote( int64_t staked ) {
      /// TODO subtract 2080 brings the large numbers closer to this decade
      double weight = int64_t( (current_time_point().sec_since_epoch() - (block_timestamp::block_timestamp_epoch / 1000)) / (seconds_per_day * 7) )  / double( 52 );
      return double(staked) * std::pow( 2, weight );
   }

   int64_t get_importance_as_stake( double social_rate, double transfer_rate, int64_t stake, int64_t total_stake ) {
      const double social_share = 0.1;
      const double transfer_share = 0.1;
      const double stake_share = 1.0 - social_share - transfer_share;
      
      /// Total importance is scaled to the value of (total_stake / stake_share), not 1.
      /// Hence the accounts with zero social and transfer rates will have importance value
      /// equal to their stake.
      
      int64_t social_as_stake = (int64_t)(total_stake / stake_share * social_share * social_rate);
      int64_t transfer_as_stake = (int64_t)(total_stake / stake_share * transfer_share * transfer_rate);
      
      int64_t importance_as_stake = stake + social_as_stake + transfer_as_stake;
      return  importance_as_stake;
   }

   double system_contract::update_total_votepay_share( const time_point& ct,
                                                       double additional_shares_delta,
                                                       double shares_rate_delta )
   {
      double delta_total_votepay_share = 0.0;
      if( ct > _gstate3.last_vpay_state_update ) {
         delta_total_votepay_share = _gstate3.total_vpay_share_change_rate
                                       * double( (ct - _gstate3.last_vpay_state_update).count() / 1E6 );
      }

      delta_total_votepay_share += additional_shares_delta;
      if( delta_total_votepay_share < 0 && _gstate2.total_producer_votepay_share < -delta_total_votepay_share ) {
         _gstate2.total_producer_votepay_share = 0.0;
      } else {
         _gstate2.total_producer_votepay_share += delta_total_votepay_share;
      }

      if( shares_rate_delta < 0 && _gstate3.total_vpay_share_change_rate < -shares_rate_delta ) {
         _gstate3.total_vpay_share_change_rate = 0.0;
      } else {
         _gstate3.total_vpay_share_change_rate += shares_rate_delta;
      }

      _gstate3.last_vpay_state_update = ct;

      return _gstate2.total_producer_votepay_share;
   }

   double system_contract::update_producer_votepay_share( const producers_table2::const_iterator& prod_itr,
                                                          const time_point& ct,
                                                          double shares_rate,
                                                          bool reset_to_zero )
   {
      double delta_votepay_share = 0.0;
      if( shares_rate > 0.0 && ct > prod_itr->last_votepay_share_update ) {
         delta_votepay_share = shares_rate * double( (ct - prod_itr->last_votepay_share_update).count() / 1E6 ); // cannot be negative
      }

      double new_votepay_share = prod_itr->votepay_share + delta_votepay_share;
      _producers2.modify( prod_itr, same_payer, [&](auto& p) {
         if( reset_to_zero )
            p.votepay_share = 0.0;
         else
            p.votepay_share = new_votepay_share;

         p.last_votepay_share_update = ct;
      } );

      return new_votepay_share;
   }

   void system_contract::voteproducer( const name& voter_name, const name& proxy, const std::vector<name>& producers ) {
      require_auth( voter_name );
      vote_stake_updater( voter_name );

      std::vector<name> calculators;
      auto calc_voter = _calc_voters.find(voter_name.value);
      if(calc_voter != _calc_voters.end()){
         calculators = calc_voter->calculators;
      }

      update_votes( voter_name, proxy, producers, true, calculators );

      auto rex_itr = _rexbalance.find( voter_name.value );
      if( rex_itr != _rexbalance.end() && rex_itr->rex_balance.amount > 0 ) {
         check_voting_requirement( voter_name, "voter holding REX tokens must vote for at least 21 producers or for a proxy" );
      }
   }

   void system_contract::votecalc(const name voter_name, const std::vector<name> &calculators) {
      require_auth( voter_name );
      vote_stake_updater( voter_name );

      std::vector<name> producers;
      name proxy;
      auto prod_voter = _voters.find(voter_name.value);
      if(prod_voter != _voters.end()){
         producers = prod_voter->producers;
         proxy = prod_voter->proxy;
      }

      update_votes( voter_name, proxy, producers, true, calculators);
   }

   void system_contract::setrates(const name voter, const double social_rate, const double transfer_rate) {
      //we require eosio authorization until the proving routine is ready
      require_auth( "eosio"_n );

      check( is_account(voter), "voter account does not exist");
      check( 0 <= social_rate && social_rate <= 1, "social rate must be in the interval from 0 to 1");
      check( 0 <= transfer_rate && transfer_rate <= 1, "transfer rate must be in the interval from 0 to 1");

      ///update rate values
      auto from_rates = _rates.find(voter.value);
      if(from_rates == _rates.end()){
         from_rates = _rates.emplace(voter, [&]( auto& v ) {
            v.owner = voter;
            v.social_rate = social_rate;
            v.transfer_rate = transfer_rate;
         });
      } else {
         _rates.modify(from_rates, same_payer, [&]( auto& v ) {
             v.social_rate = social_rate;
             v.transfer_rate = transfer_rate;
         });
      }

      ///update the votes
      auto from_voters = _voters.find(voter.value);
      if(from_voters != _voters.end()) {

         std::vector<name> calculators;
         auto calc_voter = _calc_voters.find(voter.value);
         if(calc_voter != _calc_voters.end()){
            calculators = calc_voter->calculators;
         }

         update_votes(voter, from_voters->proxy, from_voters->producers, false, calculators);
      }
   }

   void system_contract::update_votes( const name& voter_name, 
                                       const name& proxy,
                                       const std::vector<name>& producers,
                                       bool voting,
                                       const std::vector<name>& calculators ) {
      //validate input
      if ( proxy ) {
         check( producers.size() == 0, "cannot vote for producers and proxy at same time" );
         check( voter_name != proxy, "cannot proxy to self" );
      } else {
         check( producers.size() <= 30, "attempt to vote for too many producers" );
         for( size_t i = 1; i < producers.size(); ++i ) {
            check( producers[i-1] < producers[i], "producer votes must be unique and sorted" );
         }
      }
      check( calculators.size() <= 30, "attempt to vote for too many caculators" );
      for( size_t i = 1; i < calculators.size(); ++i ) {
         check( calculators[i-1] < calculators[i], "calculator votes must be unique and sorted" );
      }

      auto voter = _voters.find( voter_name.value );
      check( voter != _voters.end(), "user must stake before they can vote" ); /// staking creates voter object
      check( !proxy || !voter->is_proxy, "account registered as a proxy is not allowed to use a proxy" );

       std::vector<name> old_calculators;
       auto calc_voter = _calc_voters.find(voter_name.value);
       if(calc_voter != _calc_voters.end()){
           old_calculators = calc_voter->calculators;
       }

      /**
       * The first time someone votes we calculate and set last_vote_weight, since they cannot unstake until
       * after total_activated_stake hits threshold, we can use last_vote_weight to determine that this is
       * their first vote and should consider their stake activated.
       */
      if( voter->last_vote_weight <= 0.0 ) {
         _gstate.total_activated_stake += voter->staked;
         if( _gstate.total_activated_stake >= min_activated_stake && _gstate.thresh_activated_stake_time == time_point() ) {
            _gstate.thresh_activated_stake_time = current_time_point();
         }
      }

      ///use the rates and the stake to calculate the importance scaled to stake
      double social_rate = 0;
      double transfer_rate = 0;
      auto rate = _rates.find(voter_name.value);
      if(rate != _rates.end()) {
         social_rate = rate -> social_rate;
         transfer_rate = rate -> transfer_rate;
      }
      auto importance = get_importance_as_stake(social_rate, transfer_rate, voter->staked,
                                                _gstate.total_activated_stake);

      ///vote weight is now derived from importance, not stake
      auto new_vote_weight = stake2vote( importance );

      if( voter->is_proxy ) {
         new_vote_weight += voter->proxied_vote_weight;
      }

      std::map<name, std::pair<double, bool /*new*/> > producer_deltas;
      std::map<name, std::pair<double, bool> > calculator_deltas;
      if ( voter->last_vote_weight > 0 ) {
         if( voter->proxy ) {
            auto old_proxy = _voters.find( voter->proxy.value );
            check( old_proxy != _voters.end(), "old proxy not found" ); //data corruption
            _voters.modify( old_proxy, same_payer, [&]( auto& vp ) {
                  vp.proxied_vote_weight -= voter->last_vote_weight;
               });
            propagate_weight_change( *old_proxy );
         } else {
            for( const auto& p : voter->producers ) {
               auto& d = producer_deltas[p];
               d.first -= voter->last_vote_weight;
               d.second = false;
            }
         }
         for( const auto& c : old_calculators ) {
            auto& cd = calculator_deltas[c];
            cd.first -= voter->last_vote_weight;
            cd.second = false;
         }
      }

      if( proxy ) {
         auto new_proxy = _voters.find( proxy.value );
         check( new_proxy != _voters.end(), "invalid proxy specified" ); //if ( !voting ) { data corruption } else { wrong vote }
         check( !voting || new_proxy->is_proxy, "proxy not found" );
         if ( new_vote_weight >= 0 ) {
            _voters.modify( new_proxy, same_payer, [&]( auto& vp ) {
                  vp.proxied_vote_weight += new_vote_weight;
               });
            propagate_weight_change( *new_proxy );
         }
      } else {
         if( new_vote_weight >= 0 ) {
            for( const auto& p : producers ) {
               auto& d = producer_deltas[p];
               d.first += new_vote_weight;
               d.second = true;
            }
         }
      }
      if( new_vote_weight >= 0 ) {
         for( const auto& c : calculators ) {
            auto& cd = calculator_deltas[c];
            cd.first += new_vote_weight;
            cd.second = true;
         }
      }

      const auto ct = current_time_point();
      double delta_change_rate         = 0.0;
      double total_inactive_vpay_share = 0.0;
      for( const auto& pd : producer_deltas ) {
         auto pitr = _producers.find( pd.first.value );
         if( pitr != _producers.end() ) {
            if( voting && !pitr->active() && pd.second.second /* from new set */ ) {
               check( false, ( "producer " + pitr->owner.to_string() + " is not currently registered" ).data() );
            }
            double init_total_votes = pitr->total_votes;
            _producers.modify( pitr, same_payer, [&]( auto& p ) {
               p.total_votes += pd.second.first;
               if ( p.total_votes < 0 ) { // floating point arithmetics can give small negative numbers
                  p.total_votes = 0;
               }
               _gstate.total_producer_vote_weight += pd.second.first;
               //check( p.total_votes >= 0, "something bad happened" );
            });
            auto prod2 = _producers2.find( pd.first.value );
            if( prod2 != _producers2.end() ) {
               const auto last_claim_plus_3days = pitr->last_claim_time + microseconds(3 * useconds_per_day);
               bool crossed_threshold       = (last_claim_plus_3days <= ct);
               bool updated_after_threshold = (last_claim_plus_3days <= prod2->last_votepay_share_update);
               // Note: updated_after_threshold implies cross_threshold

               double new_votepay_share = update_producer_votepay_share( prod2,
                                             ct,
                                             updated_after_threshold ? 0.0 : init_total_votes,
                                             crossed_threshold && !updated_after_threshold // only reset votepay_share once after threshold
                                          );

               if( !crossed_threshold ) {
                  delta_change_rate += pd.second.first;
               } else if( !updated_after_threshold ) {
                  total_inactive_vpay_share += new_votepay_share;
                  delta_change_rate -= init_total_votes;
               }
            }
         } else {
            if( pd.second.second ) {
               check( false, ( "producer " + pd.first.to_string() + " is not registered" ).data() );
            }
         }
      }

      update_total_votepay_share( ct, -total_inactive_vpay_share, delta_change_rate );

      for( const auto& cd : calculator_deltas ) {
         auto citr = _calculators.find( cd.first.value );
         if( citr != _calculators.end() ) {
            check( citr->active() || !cd.second.second /* not from new set */, "producer is not currently registered" );
            _calculators.modify( citr, same_payer, [&]( auto& c ) {
               c.total_votes += cd.second.first;
               if ( c.total_votes < 0 ) { // floating point arithmetics can give small negative numbers
                  c.total_votes = 0;
               }
            });
         } else {
            check( !cd.second.second /* not from new set */, "calculator is not registered" ); //data corruption
         }
      }

      _voters.modify( voter, same_payer, [&]( auto& av ) {
         av.last_vote_weight = new_vote_weight;
         av.producers = producers;
         av.proxy     = proxy;
      });

      auto cv = _calc_voters.find( voter_name.value );
      if(cv != _calc_voters.end()) {
         _calc_voters.modify( cv, same_payer, [&]( auto& av ) {
            av.calculators = calculators;
         });
      } else {
         _calc_voters.emplace( voter_name, [&]( auto& av ) {
            av.owner = voter_name;
            av.calculators = calculators;
         });
      }
   }

   void system_contract::regproxy( const name& proxy, bool isproxy ) {
      require_auth( proxy );

      auto pitr = _voters.find( proxy.value );
      if ( pitr != _voters.end() ) {
         check( isproxy != pitr->is_proxy, "action has no effect" );
         check( !isproxy || !pitr->proxy, "account that uses a proxy is not allowed to become a proxy" );
         _voters.modify( pitr, same_payer, [&]( auto& p ) {
               p.is_proxy = isproxy;
            });
         propagate_weight_change( *pitr );
      } else {
         _voters.emplace( proxy, [&]( auto& p ) {
               p.owner  = proxy;
               p.is_proxy = isproxy;
            });
      }
   }

   void system_contract::propagate_weight_change( const voter_info& voter ) {
      check( !voter.proxy || !voter.is_proxy, "account registered as a proxy is not allowed to use a proxy" );

      ///use the rates and the stake to calculate the importance scaled to stake
      double social_rate = 0;
      double transfer_rate = 0;
      auto rate = _rates.find(voter.owner.value);
      if(rate != _rates.end()) {
         social_rate = rate -> social_rate;
         transfer_rate = rate -> transfer_rate;
      }
      auto importance = get_importance_as_stake(social_rate, transfer_rate, voter.staked,
                                                _gstate.total_activated_stake);

      ///vote weight is now derived from importance, not stake
      double new_weight = stake2vote( importance );

      if ( voter.is_proxy ) {
         new_weight += voter.proxied_vote_weight;
      }

      /// don't propagate small changes (1 ~= epsilon)
      if ( fabs( new_weight - voter.last_vote_weight ) > 1 )  {
         if ( voter.proxy ) {
            auto& proxy = _voters.get( voter.proxy.value, "proxy not found" ); //data corruption
            _voters.modify( proxy, same_payer, [&]( auto& p ) {
                  p.proxied_vote_weight += new_weight - voter.last_vote_weight;
               }
            );
            propagate_weight_change( proxy );
         } else {
            auto delta = new_weight - voter.last_vote_weight;
            const auto ct = current_time_point();
            double delta_change_rate         = 0;
            double total_inactive_vpay_share = 0;
            for ( auto acnt : voter.producers ) {
               auto& prod = _producers.get( acnt.value, "producer not found" ); //data corruption
               const double init_total_votes = prod.total_votes;
               _producers.modify( prod, same_payer, [&]( auto& p ) {
                  p.total_votes += delta;
                  _gstate.total_producer_vote_weight += delta;
               });
               auto prod2 = _producers2.find( acnt.value );
               if ( prod2 != _producers2.end() ) {
                  const auto last_claim_plus_3days = prod.last_claim_time + microseconds(3 * useconds_per_day);
                  bool crossed_threshold       = (last_claim_plus_3days <= ct);
                  bool updated_after_threshold = (last_claim_plus_3days <= prod2->last_votepay_share_update);
                  // Note: updated_after_threshold implies cross_threshold

                  double new_votepay_share = update_producer_votepay_share( prod2,
                                                ct,
                                                updated_after_threshold ? 0.0 : init_total_votes,
                                                crossed_threshold && !updated_after_threshold // only reset votepay_share once after threshold
                                             );

                  if( !crossed_threshold ) {
                     delta_change_rate += delta;
                  } else if( !updated_after_threshold ) {
                     total_inactive_vpay_share += new_votepay_share;
                     delta_change_rate -= init_total_votes;
                  }
               }
            }

            update_total_votepay_share( ct, -total_inactive_vpay_share, delta_change_rate );
         }
      }
      _voters.modify( voter, same_payer, [&]( auto& v ) {
            v.last_vote_weight = new_weight;
         }
      );
   }

} /// namespace eosiosystem

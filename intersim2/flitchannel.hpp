// $Id: flitchannel.hpp 5516 2013-10-06 02:14:48Z dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ----------------------------------------------------------------------
//
//  File Name: flitchannel.hpp
//
//  The FlitChannel models a flit channel with a multi-cycle 
//   transmission delay. The channel latency can be specified as 
//   an integer number of simulator cycles.
// ----------------------------------------------------------------------

#ifndef FLITCHANNEL_HPP
#define FLITCHANNEL_HPP

// ----------------------------------------------------------------------
//  $Author: jbalfour $
//  $Date: 2007/06/27 23:10:17 $
//  $Id: flitchannel.hpp 5516 2013-10-06 02:14:48Z dub $
// ----------------------------------------------------------------------

#include "channel.hpp"
#include "flit.hpp"

#include <boost/dynamic_bitset.hpp>	//added by kh(061715)
using namespace std;

class Router ;

//added by kh(061715)
//Hai
class HistoryBitSet{
private:
  boost::dynamic_bitset<> *_history;
  int history_size;
  int currentPos;
public:
  HistoryBitSet(int size);
  ~HistoryBitSet();
  inline int getHistorySize(){return history_size;}
  void setLinkUsed();
  void unSetLinkUsed();
  double getLinkUsageFrequency(int atTime);
};
//End Hai

class FlitChannel : public Channel<Flit> {
public:
  FlitChannel(Module * parent, string const & name, int classes);

  void SetSource(Router const * const router, int port) ;
  inline Router const * const GetSource() const {
    return _routerSource;
  }
  inline int const & GetSourcePort() const {
    return _routerSourcePort;
  }
  void SetSink(Router const * const router, int port) ;
  inline Router const * const GetSink() const {
    return _routerSink;
  }
  inline int const & GetSinkPort() const {
    return _routerSinkPort;
  }
  inline vector<int> const & GetActivity() const {
    return _active;
  }

  //added by kh(061715)
	//Hai
	  inline const int& getIdle()const{
		return _idle;
	  }
	  inline const int getLinkWidth()const{
		  return _linkWidthLevel;
	  }
	  inline const bool isInMaxSpeed()const{
		  return (_linkWidthLevel==_maxLinkWidthLevel);
	  }
	  inline const bool isDescendingLink()const{
		return _isDescendingLink;
	  }
	  inline const unsigned long long getNumBusyCycles()const{
		  return _numBusyCycles;
	  }
	  double getLinkUsageFrequency(int atTime);
	  const int getNumLinkWidthChanges() const;
	  inline const int getNumLinkWidthIncreaseWithNoCongest(){return _numLinkWidthIncreaseWithNoCongest;}
	  inline const int getNumLinkWidthDecreaseWithNoCongest(){return _numLinkWidthDecreaseWithNoCongest;}
	  inline const int getNumLinkWidthIncreaseWithCongest(){return _numLinkWidthIncreaseWithCongest;}
	  inline const int getNumLinkWidthDecreaseWithCongest(){return _numLinkWidthDecreaseWithCongest;}
	  inline const bool getIsEverMaximumWidth(){
		return _isEverMaximumWidth;
	  }
	  double getEnergyPercentageUsed(int atTime);
	  double getRoutingPenalizingCoefficient();
	  double getPercentageBufferOccupancyOfTheNextRouter();
	  void setEverMaximumWidth(bool isEver);
	  void setDescendingLink(bool isDescendingLink);

	  void setNumBusyCycles(unsigned long long cycle) 	{	_numBusyCycles = cycle;   						}
	  void addTotNumBusyCycles(unsigned long long cycle) 	{ 	_totNumBusycycles = _totNumBusycycles + cycle;	}
	  unsigned long long getTotNumBusyCycles()			{	return	_totNumBusycycles;						}
   //End Hai




  // Send flit 
  virtual void Send(Flit * flit);

  virtual void ReadInputs();
  virtual void WriteOutputs();

private:
  
  ////////////////////////////////////////
  //
  // Power Models OBSOLETE
  //
  ////////////////////////////////////////

  Router const * _routerSource;
  int _routerSourcePort;
  Router const * _routerSink;
  int _routerSinkPort;

  // Statistics for Activity Factors
  vector<int> _active;
  int _idle;


  //added by kh(061715)
  //Hai
  HistoryBitSet* _historyBitSet;
  int _hai_lu_history_size;
  int _decision_period;
  //If the time takes to transfer a flit equal x cycles - x is greater than 1,
  //then for the next x-1 cycle time don't mark that the link was not used because actually it is used,
  //just decrease this variable in that case
  int _cyclesOwedFromLastFlit;
  int _linkWidthLevel;
  int _pendingLinkWidthLevel;
  bool _isPendingForLinkWidthIncrease;
  int _timeToReleasePendingLinkWidthLevel;
  int _maxLinkWidthLevel;
  int _link_changing_status_delay;
  double _low_lu_threshold_no_congest;
  double _hi_lu_threshold_no_congest;
  double _low_lu_threshold_with_congest;
  double _hi_lu_threshold_with_congest;
  double _buffer_occupancy_threshold;
  bool _isDownstreamCongested;
  int _numLinkWidthIncreaseWithNoCongest;
  int _numLinkWidthDecreaseWithNoCongest;
  int _numLinkWidthIncreaseWithCongest;
  int _numLinkWidthDecreaseWithCongest;
  int _proportionalEnergyConsumed;
  double _linkwidth_reduction_routing_penalty;
  unsigned long long _numBusyCycles;
  unsigned long long _totNumBusycycles;
  bool _isEverMaximumWidth;
  bool _isDescendingLink;
  int _minLinkWidthLevel;
  //Method
  int _getLatencyForLinkWidthLevel(int level);
  void _increaseProportionalEnergyConsumedForThisCycle();
  void _proceedDecreaseLinkWidth(int simTime,bool isCongestedSituation);
  void _proceedIncreaseLinkWidth(int simTime,bool isCongestedSituation);
  //End Hai

  //added by kh(122815)
  int _subnet_id;
  int _channel_id;
  ///




};

#endif

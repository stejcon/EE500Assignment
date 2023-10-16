/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Joe Kopena <tjkopena@cs.drexel.edu>
 * Modified by: Longhao Zou, Oct 2016 for EE500 <longhao.zou@dcu.ie>
 *

            EE500 Assignment 2016-2017
            Default WiFi Network Topology

                WiFi 192.168.0.0
            -------------------------
            |AP (node 0:192.168.0.1)|
            -------------------------
             *         *           *
            /          |            \
  Traffic 1/  Traffic 2|    ------   \ Traffic N
          /            |              \
      user 1       user 2     ------   user N
   (node 1         (node 2     ------  (node N
   :192.168.0.2    :192.168.0.3 ------ :192.168.0.N+1
   :1000)          :1001)        ------:1000+(N-1))

   PS: In this example, I just implemented only 1 WiFi user.

 */

#include <ctime>
#include <iostream>
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"

#include "ns3/stats-module.h"

#include "wifi-example-apps.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WiFiExampleSim");




void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,
                 std::string path, Ptr<const Packet> packet) {
  NS_LOG_INFO ("Sent frame counted in " <<
               datac->GetKey ());
  datac->Update ();
  // end TxCallback
}




//----------------------------------------------------------------------
//-- main
//----------------------------------------------
int main (int argc, char *argv[]) {
  LogComponentEnable ("WiFiDistanceApps", LOG_LEVEL_INFO);
  double distance = 50.0;
  double simTime = 20; //Simulation Running Time (in seconds)
  string format ("db"); //Default as .db format (Please refer to sqlite-data-output.cc and sqlite-data-output.h located in /src/stats/model)

  string experiment ("wifi-example-sim"); //the name of your experiment
  string strategy ("wifi-default");
  string input;
  string runID;

  {
    stringstream sstr;
    sstr << "run-" << time (NULL);
    runID = sstr.str ();
  }

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue ("distance", "Distance apart to place nodes (in meters).",
                distance);
  //cmd.AddValue ("format", "Format to use for data output.",
    //            format);
  cmd.AddValue ("simTime", "Simulation Running Time (in seconds)", simTime);
  cmd.AddValue ("experiment", "Identifier for experiment.",
                experiment);
  cmd.AddValue ("strategy", "Identifier for strategy.",
                strategy);
  cmd.AddValue ("run", "Identifier for run.",
                runID);
  cmd.Parse (argc, argv);

  if (format != "omnet" && format != "db") {
      NS_LOG_ERROR ("Unknown output format '" << format << "'");
      return -1;
    }

  #ifndef STATS_HAS_SQLITE3
  if (format == "db") {
      return -1;
    }
  #endif

  {
    stringstream sstr ("");
    sstr << distance;
    input = sstr.str ();
  }




  //------------------------------------------------------------
  //-- Create nodes and network stacks
  //--------------------------------------------
  NS_LOG_INFO ("Creating nodes.");
  NodeContainer nodes;
  nodes.Create (2);

  NS_LOG_INFO ("Installing WiFi and Internet stack.");
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211ac); // Try to run with different WiFI standard like WIFI_PHY_STANDARD_80211ax (802.11ac operates in the 5Ghz range only, while 802.11ax operates in both the 2.4Ghz and 5Ghz ranges, thus creating more available channels. ... 802.11ax supports up to eight MU-MIMO transmissions at a time, up from four with 802.11ac)
  wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set ("TxGain", DoubleValue(20.0));
  NetDeviceContainer nodeDevices = wifi.Install (wifiPhy, wifiMac, nodes);

  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  ipAddrs.Assign (nodeDevices);

  //------------------------------------------------------------
  //-- Setup physical layout
  //--------------------------------------------
  NS_LOG_INFO ("Installing static mobility; distance " << distance << " .");
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
    CreateObject<ListPositionAllocator>();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, distance, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (nodes);


  //------------------------------------------------------------
  //-- Create the traffic between AP and WiFi Users
  //------------------------------------------------------------
  //------------------------------------------------------------
  //-- 1. Create the first traffic for the first WiFi user on WiFi AP (source)
  //--------------------------------------------
  NS_LOG_INFO ("Create traffic source & sink.");
  Ptr<Node> appSource = NodeList::GetNode (0);
  Ptr<Sender> sender = CreateObject<Sender>();
  sender->SetAttribute("Port", UintegerValue(1000));//Lisening Port of the first WiFi user
  sender->SetAttribute("PacketSize", UintegerValue (1000)); //bytes
  sender->SetAttribute("Interval", StringValue ("ns3::ConstantRandomVariable[Constant=0.05]")); //seconds
  sender->SetAttribute("NumPackets",UintegerValue (100000000));
  appSource->AddApplication (sender);
  sender->SetStartTime (Seconds (0));

  //------------------------------------------------------------
  //-- 2. Create the first WiFi User (sink)
  //--------------------------------------------
  Ptr<Node> appSink = NodeList::GetNode (1);
  Ptr<Receiver> receiver = CreateObject<Receiver>();
  receiver->SetAttribute("Port", UintegerValue(1000));//Lisening Port
  appSink->AddApplication (receiver);
  receiver->SetStartTime (Seconds (0));

  //Set IP address of the first User to AP (source)
  Config::Set ("/NodeList/*/ApplicationList/*/$Sender/Destination",
               Ipv4AddressValue ("192.168.0.2"));




  //------------------------------------------------------------
  //-- Setup stats and data collection
  //--  for the WiFi AP and the first WiFi User
  //--------------------------------------------

  // Create a DataCollector object to hold information about this run.
  DataCollector dataofuser1;
  dataofuser1.DescribeRun (experiment,
                    strategy,
                    input,
                    runID);

  // Add any information we wish to record about this run.
  dataofuser1.AddMetadata ("author", "EE500-XXX"); //Please replace XXX with your first name!


  // Create a counter to track how many frames are generated.  Updates
  // are triggered by the trace signal generated by the WiFi MAC model
  // object.  Here we connect the counter to the signal via the simple
  // TxCallback() glue function defined above.
  Ptr<CounterCalculator<uint32_t> > totalTx =
    CreateObject<CounterCalculator<uint32_t> >();
  totalTx->SetKey ("wifi-tx-frames");
  totalTx->SetContext ("node[0]");
  Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                   MakeBoundCallback (&TxCallback, totalTx));
  dataofuser1.AddDataCalculator (totalTx);

  // This is similar, but creates a counter to track how many frames
  // are received.  Instead of our own glue function, this uses a
  // method of an adapter class to connect a counter directly to the
  // trace signal generated by the WiFi MAC.
  Ptr<PacketCounterCalculator> totalRx =
    CreateObject<PacketCounterCalculator>();
  totalRx->SetKey ("wifi-rx-frames");
  totalRx->SetContext ("node[1]");
  Config::Connect ("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
                   MakeCallback (&PacketCounterCalculator::PacketUpdate,
                                 totalRx));
  dataofuser1.AddDataCalculator (totalRx);




  // This counter tracks how many packets---as opposed to frames---are
  // generated.  This is connected directly to a trace signal provided
  // by our Sender class.
  Ptr<PacketCounterCalculator> appTx =
    CreateObject<PacketCounterCalculator>();
  appTx->SetKey ("sender-tx-packets");
  appTx->SetContext ("node[0]");
  Config::Connect ("/NodeList/0/ApplicationList/*/$Sender/Tx",
                   MakeCallback (&PacketCounterCalculator::PacketUpdate,
                                 appTx));
  dataofuser1.AddDataCalculator (appTx);

  // Here a counter for received packets is directly manipulated by
  // one of the custom objects in our simulation, the Receiver
  // Application.  The Receiver object is given a pointer to the
  // counter and calls its Update() method whenever a packet arrives.
  Ptr<CounterCalculator<> > appRx =
    CreateObject<CounterCalculator<> >();
  appRx->SetKey ("receiver-rx-packets");
  appRx->SetContext ("node[1]");
  receiver->SetCounter (appRx);
  dataofuser1.AddDataCalculator (appRx);




  /**
   * Just to show this is here...
   Ptr<MinMaxAvgTotalCalculator<uint32_t> > test =
   CreateObject<MinMaxAvgTotalCalculator<uint32_t> >();
   test->SetKey("test-dc");
   data.AddDataCalculator(test);

   test->Update(4);
   test->Update(8);
   test->Update(24);
   test->Update(12);
  **/

  // This DataCalculator connects directly to the transmit trace
  // provided by our Sender Application.  It records some basic
  // statistics about the sizes of the packets received (min, max,
  // avg, total # bytes), although in this scenaro they're fixed.
  Ptr<PacketSizeMinMaxAvgTotalCalculator> appTxPkts =
    CreateObject<PacketSizeMinMaxAvgTotalCalculator>();
  appTxPkts->SetKey ("tx-pkt-size");
  appTxPkts->SetContext ("node[0]");
  Config::Connect ("/NodeList/0/ApplicationList/*/$Sender/Tx",
                   MakeCallback
                     (&PacketSizeMinMaxAvgTotalCalculator::PacketUpdate,
                     appTxPkts));
  dataofuser1.AddDataCalculator (appTxPkts);


  // Here we directly manipulate another DataCollector tracking min,
  // max, total, and average propagation delays.  Check out the Sender
  // and Receiver classes to see how packets are tagged with
  // timestamps to do this.
  Ptr<TimeMinMaxAvgTotalCalculator> delayStat =
    CreateObject<TimeMinMaxAvgTotalCalculator>();
  delayStat->SetKey ("delay");
  delayStat->SetContext (".");
  receiver->SetDelayTracker (delayStat); //nanoseconds
  dataofuser1.AddDataCalculator (delayStat);




  //------------------------------------------------------------
  //-- Run the simulation
  //--------------------------------------------
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(simTime));
  Simulator::Run ();




  //------------------------------------------------------------
  //-- Generate statistics output.
  //--------------------------------------------

  // Pick an output writer based in the requested format.
  Ptr<DataOutputInterface> output = 0;
  if (format == "omnet") {
      NS_LOG_INFO ("Creating omnet formatted data output.");
      output = CreateObject<OmnetDataOutput>();
    } else if (format == "db") {
    #ifdef STATS_HAS_SQLITE3
      output = CreateObject<SqliteDataOutput>();
    #endif
    } else {
      NS_LOG_ERROR ("Unknown output format " << format);
    }

  // Finally, have that writer interrogate the DataCollector and save
  // the results.
  if (output != 0)
  {
    output->SetFilePrefix("DataOfUser1");
    output->Output (dataofuser1);
  }
  // Free any memory here at the end of this example.

  Simulator::Destroy ();

  // end main
}

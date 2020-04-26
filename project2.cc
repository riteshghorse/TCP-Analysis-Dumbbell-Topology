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
 */



/*
          Network Topology
    
  n0 ----               ---- n4
         -             -
          n2---------n3
         -             -
  n1 ----               ---- n5
    


*/
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpAnalysis");


void exp1 (Ptr<Node> src, Ptr<Node> dest, Address sinkAddress, uint16_t sinkPort, std::string tcp_version, double startTime, double endTime)
{
    // tcp from no to n4
  if(tcp_version.compare ("TcpBic"))
  {
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBic::GetTypeId()));
  }
  else if (tcp_version.compare ("TcpDctcp"))
  {
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpDctcp::GetTypeId()));
  }
  else
  {
    exit(EXIT_FAILURE);
  }
  
  uint maxBytes = 1 * 1024;
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sinkPort));
  ApplicationContainer sinkApp = packetSinkHelper.Install (dest);
  sinkApp.Start (Seconds (startTime));
  sinkApp.Stop (Seconds (endTime));

  // modified code for blk send start----------------------------

  BulkSendHelper sourceHelper ("ns3::TcpSocketFactory", sinkAddress);
  sourceHelper.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApp = sourceHelper.Install (src);
  sourceApp.Start (Seconds (startTime));
  sourceApp.Stop (Seconds (endTime));
  // modified code for blk send end---------------------------------
}

double getStandardDeviation (double arr[], double mean, int start, int end)
{
  double sdv = 0.0;
  for(int i=start; i<end; ++i)
    sdv += pow(arr[i]-mean, 2);
  
  return sqrt (sdv / 3);
}
void writeToFile(int index, std::vector<double> v, std::vector<double> fctv)
{
  int sum = 0;
  double avg, sdv;
  double avg2, sdv2;
  std::ofstream file, file2;
  file.open ("tcp_rghorse.csv", std::ios_base::app);
  file2.open ("achfile.csv", std::ios_base::app);
  if (v.size () == 6)
  {
    double arr[] = {v[0], v[2], v[4]};
    double farr[] = {fctv[0], fctv[2], fctv[4]};
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += arr[i];
    avg = sum/3;
    sdv = getStandardDeviation(arr, avg, 0, 3);
    file << "th_" << index << "," << arr[0] << "," << arr[1] << "," << arr[2] << "," << avg << "," << sdv << "," << "Mbps,,,,,,\n" ;
    file.close();
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += farr[i];
    avg = sum/3;
    sdv = getStandardDeviation(farr, avg, 0, 3);
    file2 << "afct_" << index << "," << farr[0] << "," << farr[1] << "," << farr[2] << "," << avg << "," << sdv << "," << "sec,,,,,,\n" ;
    file2.close();
    return;
  }
  if(v.size() == 12)
  {
    double arr[] = {v[0], v[4], v[8], v[1], v[5], v[9]};
    double farr[] = {fctv[0], fctv[4], fctv[8], fctv[1], fctv[5], fctv[9]};
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += arr[i];
    avg = sum/3;
    sdv = getStandardDeviation(arr, avg, 0, 3);
    sum = 0;
    for(int i=3; i<6; ++i)
      sum += arr[i];
    avg2 = sum/3;
    sdv2 = getStandardDeviation(arr, avg2, 3, 6);
    file << "th_" << index << "," << arr[0] << "," << arr[1] << "," << arr[2] << "," << avg << "," << sdv << "," << "Mbps," << arr[3] << "," << arr[4] << "," << arr[5] << "," << avg2 << "," << sdv2 << "," << "Mbps\n" ;
    file.close();
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += farr[i];
    avg = sum/3;
    sdv = getStandardDeviation(farr, avg, 0, 3);
    sum = 0;
    for(int i=3; i<6; ++i)
      sum += farr[i];
    avg2 = sum/3;
    sdv2 = getStandardDeviation(farr, avg2, 3, 6);
    file2 << "afct_" << index << "," << farr[0] << "," << farr[1] << "," << farr[2] << "," << avg << "," << sdv << "," << "sec," << farr[3] << "," << farr[4] << "," << farr[5] << "," << avg2 << "," << sdv2 << "," << "sec\n" ;
    file.close();
    return;
  }
}

void mergeFiles()
{
  std::ifstream in("achfile.csv");
  std::ofstream out("tcp_rghorse.csv", std::ios_base::out | std::ios_base::app);

  for (std::string str; std::getline(in, str); )
  {
      out << str << "\n";
  }
}

int main (int argc, char *argv[])
{
  CommandLine cmd;

  cmd.Parse (argc, argv);
  // uint32_t maxBytes = 1 * 1024 * 1024;
  // uint32_t packetSize = 1.2 * 1024;
  // uint32_t numPackets = maxBytes / packetSize;
  double startTime, endTime, gapTime=10.0;
  double thro;  
  int index, value, k_index;
  std::vector<double> thro_v, ft_v;
  std::map<int, int> results;

  results.insert ( std::pair<int, int>(1, 6));
  results.insert ( std::pair<int, int>(2, 12));
  results.insert ( std::pair<int, int>(3, 6));
  results.insert ( std::pair<int, int>(4, 12));
  results.insert ( std::pair<int, int>(5, 12));

  Time::SetResolution (Time::NS); 

  NS_LOG_INFO ("Creating Nodes");
  NodeContainer nodes;
  nodes.Create (6);

  NodeContainer n0n2 = NodeContainer (nodes.Get (0), nodes.Get (2));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));
  NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5));

  // install protocols
  InternetStackHelper internet;
  internet.Install (nodes);

  // create channels
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // creating channels
  NetDeviceContainer d0d2 = pointToPoint.Install (n0n2);
  NetDeviceContainer d1d2 = pointToPoint.Install (n1n2);
  NetDeviceContainer d2d3 = pointToPoint.Install (n2n3);
  NetDeviceContainer d3d4 = pointToPoint.Install (n3n4);
  NetDeviceContainer d3d5 = pointToPoint.Install (n3n5); 

  // adding IP addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = address.Assign (d0d2);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = address.Assign (d1d2);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = address.Assign (d2d3);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = address.Assign (d3d4);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = address.Assign (d3d5);


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // for netanim---------------------------------------------------
  AnimationInterface anim ("project.xml");
  anim.SetConstantPosition (nodes.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (nodes.Get(1), 10.0, 70.0);
  anim.SetConstantPosition (nodes.Get(2), 40.0, 40.0);
  anim.SetConstantPosition (nodes.Get(3), 60.0, 40.0);
  anim.SetConstantPosition (nodes.Get(4), 80.0, 10.0);
  anim.SetConstantPosition (nodes.Get(5), 80.0, 70.0);


  
  uint16_t sinkPort_1 = 8080;
  Address sinkAddress_1 (InetSocketAddress (i3i4.GetAddress (1), sinkPort_1));
  
  uint16_t sinkPort_2 = 8080;
  Address sinkAddress_2 (InetSocketAddress (i3i5.GetAddress (1), sinkPort_2));
  
  startTime = 0.0;
  endTime = 10.0;
  int i = 3;

  // Experiment 1
  while (i--)
  {
    exp1 (nodes.Get (0), nodes.Get (4), sinkAddress_1, sinkPort_1, "TcpBic", startTime, endTime);
    startTime += gapTime + 1;
    endTime += gapTime;
  }

  // Experiment 2
  i = 3;
  while (i--)
  {
    exp1 (nodes.Get (0), nodes.Get (4), sinkAddress_1, sinkPort_1, "TcpBic", startTime, endTime);
    exp1 (nodes.Get (1), nodes.Get (5), sinkAddress_2, sinkPort_2, "TcpBic", startTime, endTime);
    startTime += gapTime;
    endTime += gapTime;
  }

  // Experiment 3
  i = 3;
  while (i--)
  {
    exp1 (nodes.Get (0), nodes.Get (4), sinkAddress_1, sinkPort_1, "TcpDctcp", startTime, endTime);
    startTime += gapTime + 1;
    endTime += gapTime;
  }


  // Experiment 4
  i = 3;
  while (i--)
  {
    exp1 (nodes.Get (0), nodes.Get (4), sinkAddress_1, sinkPort_1, "TcpDctcp", startTime, endTime);
    exp1 (nodes.Get (1), nodes.Get (5), sinkAddress_2, sinkPort_2, "TcpDctcp", startTime, endTime);
    startTime += gapTime;
    endTime += gapTime;
  }

  // Experiment 5
  i = 3;
  while (i--)
  {
    exp1 (nodes.Get (0), nodes.Get (4), sinkAddress_1, sinkPort_1, "TcpBic", startTime, endTime);
    exp1 (nodes.Get (1), nodes.Get (5), sinkAddress_2, sinkPort_2, "TcpDctcp", startTime, endTime);
    startTime += gapTime;
    endTime += gapTime;
  }
  // Both the destination nodes have started thier service at this point


  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds (240.));
  Simulator::Run ();
  Simulator::Destroy();
  
  
  // adding code for flow monitor ------------------------

  // FlowMonitorHelper flowmon;
  // Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  //code for output
  index = 1;
  value = results[index];
  k_index = 0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i!=stats.end (); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    // if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress=="10.1.4.2"))
    // {
      std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
    // }
    thro = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
    thro_v.push_back(thro);
    double ft = i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
    ft_v.push_back(ft);
    ++k_index;
    // std::cout << "ft: " << ft << "\n";
    if(k_index == value)
    {
      k_index = 0;
      writeToFile(index, thro_v, ft_v);
      thro_v.clear();
      ++index;
      value = results[index];
    }
  }

  monitor->SerializeToXmlFile ("project-2.flowmon", true, true);
  // adding code for flow monitor ------------------------
  
  mergeFiles();

  
  // Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps_1[0].Get (0));
  // std::cout << "total bytes rcvd : " << sink1->GetTotalRx () << std::endl;

  // Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApps_2.Get (0));
  // std::cout << "total bytes rcvd : " << sink2->GetTotalRx () << std::endl;
  return 0;
}

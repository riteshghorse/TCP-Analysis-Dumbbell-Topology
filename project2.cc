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


class App : public Application
{
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void ScheduleTx ();
    void SendPacket ();

    Ptr<Socket>     m_socket;
    Address         m_peer;
    uint32_t        m_packetSize;
    uint32_t        m_n_Packets;
    DataRate        m_dataRate;
    EventId         m_sendEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
  public:
    App ();
    virtual ~App ();
    void Init (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
    void ChangeRate (DataRate rate);
    void Receive (int nbytes);
};

App::App () : m_socket (0), m_peer (), m_packetSize (0), m_n_Packets (0), m_dataRate (0),
    m_sendEvent (), m_running (false), m_packetsSent (0){
}

App::~App ()
{
  m_socket = 0;
}

void App::Init (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_n_Packets = nPackets;
  m_dataRate = dataRate;
}

void App::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void App::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    Simulator::Cancel (m_sendEvent);

  if (m_socket)
    m_socket->Close ();
}

void App::SendPacket ()
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_n_Packets)
    ScheduleTx ();  
}

void App::ScheduleTx ()
{
  if (m_running)
  {
    Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
    m_sendEvent = Simulator::Schedule (tNext, &App::SendPacket, this);
  }
}

void App::ChangeRate (DataRate rate)
{
  m_dataRate = rate;
}

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
  
  uint maxBytes = 500 * 1024 * 1024;
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

double getStandardDeviation (std::vector<double> v, double mean, int start)
{
  double sdv = 0.0;
  for(int i=start; i<start+3; ++i)
    sdv += pow(v[i]-mean, 2);
  
  return sqrt (sdv / 3);
}
void writeToFile(int index, std::vector<double> v)
{
  int sum = 0;
  double avg, sdv;
  double avg2, sdv2;
  std::ofstream file;
  file.open("tcp_rghorse.csv", std::ios_base::app);
  if(v.size() == 3)
  {
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += v[i];
    avg = sum/3;
    sdv = getStandardDeviation(v, avg, 0);
    file << "th_" << index << "," << v[0] << "," << v[1] << "," << v[2] << "," << avg << "," << sdv << "," << "Mbps,,,,,,\n" ;
    file.close();
    return;
  }
  if(v.size() == 6)
  {
    sum = 0;
    for(int i=0; i<3; ++i)
      sum += v[i];
    avg = sum/3;
    sdv = getStandardDeviation(v, avg, 0);
    sum = 0;
    for(int i=3; i<6; ++i)
      sum += v[i];
    avg2 = sum/3;
    sdv2 = getStandardDeviation(v, avg2, 3);
    file << "th_" << index << "," << v[0] << "," << v[1] << "," << v[2] << "," << avg << "," << sdv << "," << "Mbps," << v[3] << "," << v[4] << "," << v[5] << "," << avg2 << "," << sdv2 << "," << "Mbps\n" ;
    file.close();
    return;
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
  std::vector<double> thro_v;
  std::map<int, int> results;


  results.insert ( std::pair<int, int>(1, 3));
  results.insert ( std::pair<int, int>(2, 6));
  results.insert ( std::pair<int, int>(3, 3));
  results.insert ( std::pair<int, int>(4, 6));
  results.insert ( std::pair<int, int>(5, 6));

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
    ++k_index;
    if(k_index == value)
    {
      k_index = 0;
      writeToFile(index, thro_v);
      thro_v.clear();
      ++index;
      value = results[index];
    }
  }

  monitor->SerializeToXmlFile ("project-2.flowmon", true, true);
  // adding code for flow monitor ------------------------
  
  
  
  
  // Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps_1[0].Get (0));
  // std::cout << "total bytes rcvd : " << sink1->GetTotalRx () << std::endl;

  // Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApps_2.Get (0));
  // std::cout << "total bytes rcvd : " << sink2->GetTotalRx () << std::endl;
  return 0;
}

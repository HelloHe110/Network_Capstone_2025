#include <iostream>
#include <fstream>
#include <map>
#include <queue>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/leo-module.h"

using namespace ns3;
using namespace std;

int port = 9;
NodeContainer satellites;
NodeContainer groundStations;
NetDeviceContainer utNet;
ApplicationContainer sinkApps;

map<pair<int, int>, string> linkRates;
map<int, vector<int>> satToGsQueue;
map<int, vector<int>> assocMap;
map<int, double> gsStartTime, gsEndTime;
map<int, double> satFinishTime, satCollectTime;
map<int, bool> gsStarted, satBusy;
string outputFile;
std::string ns3_path_head = "contrib/leo/examples/";
std::map<int, int> sat_cnt;

typedef pair<int, int> GsSatPair;
int SIZEEE = 125000 * 1;

string GetNodeId(string str) {
  size_t pos1 = str.find("/", 0);
  size_t pos2 = str.find("/", pos1 + 1);
  size_t pos3 = str.find("/", pos2 + 1);
  return str.substr(pos2 + 1, pos3 - pos2 - 1);
}

void ParseLinkRates(string filename) {
  ifstream in(ns3_path_head + filename);
  int numGs, numSat, dummy;
  in >> numGs >> numSat >> dummy;
  
  int gs, sat, rate;
  while (in >> gs >> sat >> rate) {
    // printf("gs: %d, sat: %d, rate: %d\n", gs, sat, rate);
    linkRates[make_pair(gs, sat)] = to_string(rate) + "kbps";
  }
}


void ParseAssociation(string filename) {
  // printf("============================================\n");
  ifstream in(ns3_path_head + filename);
  double dummy;
  in >> dummy;
  int gs, sat, cnt = 0;
  while (in >> gs >> sat) {
    if (cnt >= 20) break;
    cnt++;
    satToGsQueue[sat].push_back(gs);
    assocMap[sat].push_back(gs);
    gsStarted[gs] = false;
    // printf("gs: %d, sat: %d\n", gs, sat);
  }
  // printf("============================================\n");
}

void SendPacket(int gsId, int satId) {
  Ptr<Node> gsNode = groundStations.Get(gsId);
  Ptr<Node> satNode = satellites.Get(satId);

  sat_cnt[satId]++;

  // utNet.Get(gsNode->GetId())->SetAttribute("DataRate", StringValue(linkRates[make_pair(gsId, satId)]));
  utNet.Get(gsNode->GetId())->GetObject<MockNetDevice>()->SetDataRate(linkRates[make_pair(gsId, satId)]);
  // utNet.Get(satNode->GetId())->SetAttribute("DataRate", StringValue(linkRates[make_pair(gsId, satId)]));
  utNet.Get(satNode->GetId())->GetObject<MockNetDevice>()->SetDataRate(linkRates[make_pair(gsId, satId)]);

  Ipv4Address dstAddr = satNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

  BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(dstAddr, port));
  source.SetAttribute("MaxBytes", UintegerValue(SIZEEE));
  source.SetAttribute("SendSize", UintegerValue(512));

  ApplicationContainer app = source.Install(gsNode);
  // app.Start(Seconds(Simulator::Now().GetSeconds()));
  app.Start(Seconds(0));

  gsStartTime[gsId] = Simulator::Now().GetSeconds();
  gsStarted[gsId] = true;
  satBusy[satId] = true;
}

static void EchoRx(std::string context, const Ptr<const Packet> packet, const TcpHeader &header, const Ptr<const TcpSocketBase> socket) {
  int nodeId = stoi(GetNodeId(context));
  if (nodeId >= (int)satellites.GetN()) return;

  // Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps.Get(nodeId));
  Ptr<PacketSink> sink = DynamicCast<PacketSink>(satellites.Get(nodeId)->GetApplication(0));
  if (!sink) return;
  uint64_t totalRx = sink->GetTotalRx();
  if (totalRx < (uint64_t)(SIZEEE * (sat_cnt[nodeId]))) return;

  for (int gsId : assocMap[nodeId]) {
    if (gsEndTime.find(gsId) == gsEndTime.end() && gsStartTime.count(gsId)) {
      gsEndTime[gsId] = Simulator::Now().GetSeconds();
      satFinishTime[nodeId] = gsEndTime[gsId];
      satCollectTime[nodeId] += gsEndTime[gsId] - gsStartTime[gsId];
      printf("nodeId: %s from %s\n", GetNodeId(context).c_str(), context.c_str());
      printf("Satellite %d finished receiving data at time: %f\n", nodeId, satFinishTime[nodeId]);
      break;
    }
  }

  for (int gsId : assocMap[nodeId]) {
    if (gsStartTime.find(gsId) == gsStartTime.end()) {
      Simulator::ScheduleNow(&SendPacket, gsId, nodeId);
      break;
    }
  }
}

void Connect() {
  Config::Connect("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx", MakeCallback(&EchoRx));
}

int main(int argc, char *argv[]) {
  string constellation = "TelesatGateway";
  double duration = 100;
  string inputFile = "network.ortools.out";
  string graphFile = "network.graph";

  CommandLine cmd;
  cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
  cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
  cmd.AddValue("inputFile", "Input file", inputFile);
  cmd.AddValue("outputFile", "Output file", outputFile);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(2.0)));

  LeoOrbitNodeHelper orbit;
  satellites = orbit.Install({LeoOrbit(1200, 20, 1, 60)});

  LeoGndNodeHelper ground;
  ground.Add(groundStations, LeoLatLong(20, 4));
  ground.Add(groundStations, LeoLatLong(19, 12));
  ground.Add(groundStations, LeoLatLong(19, 10));
  ground.Add(groundStations, LeoLatLong(19, 19));
  ground.Add(groundStations, LeoLatLong(19, 20));
  ground.Add(groundStations, LeoLatLong(18, 20));
  ground.Add(groundStations, LeoLatLong(18, 22));
  ground.Add(groundStations, LeoLatLong(17, 26));
  ground.Add(groundStations, LeoLatLong(18, 30));
  ground.Add(groundStations, LeoLatLong(15, 40));
  ground.Add(groundStations, LeoLatLong(14, 25));
  ground.Add(groundStations, LeoLatLong(14, 30));
  ground.Add(groundStations, LeoLatLong(14, 40));
  ground.Add(groundStations, LeoLatLong(14, 50));
  ground.Add(groundStations, LeoLatLong(14, 52));
  ground.Add(groundStations, LeoLatLong(13, 50));
  ground.Add(groundStations, LeoLatLong(13, 48));
  ground.Add(groundStations, LeoLatLong(12, 50));
  ground.Add(groundStations, LeoLatLong(13, 52));
  ground.Add(groundStations, LeoLatLong(15, 30));

  ParseLinkRates(graphFile);
  ParseAssociation(inputFile);

  LeoChannelHelper utCh;
  utCh.SetConstellation(constellation);
  utNet = utCh.Install(satellites, groundStations);

  AodvHelper aodv;
  aodv.Set("EnableHello", BooleanValue(false));

  InternetStackHelper stack;
  stack.SetRoutingHelper(aodv);
  stack.Install(satellites);
  stack.Install(groundStations);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.0.0", "255.255.0.0");
  ipv4.Assign(utNet);

  PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  for (int i = 0; i < 60; ++i) {
    sinkApps.Add(sink.Install(satellites.Get(i)));
  }

  for (auto &[sat, q] : satToGsQueue) {
    // auto it = min_element(q.begin(), q.end());
    if (!q.empty() && !gsStarted[q.front()]) {
      // Simulator::Schedule(Seconds(0), &SendPacket, q.front(), sat);
      SendPacket(q.front(), sat);
      // printf("Sending packet from gs%d to sat%d\n", *it, sat);
    }
  }

  Simulator::Schedule(Seconds(1e-7), &Connect);
  Simulator::Stop(Seconds(duration));
  Simulator::Run();
  Simulator::Destroy();

  ofstream out(ns3_path_head + outputFile);
  double totalTime = 0;
  for (auto &[_, t] : gsEndTime) totalTime = max(totalTime, t);
  out << totalTime << endl;
  for (int i = 0; i < 60; ++i) {
    if (satCollectTime.count(i)) {
      out << i << " " << satCollectTime[i] << endl;
    } else {
      out << i << " 0" << endl;
    }
  }
  for (int gs = 0; gs < 20; ++gs) {
    if (gsStartTime.count(gs) && gsEndTime.count(gs)) {
      out << gs << " " << gsStartTime[gs] << " " << gsEndTime[gs] << endl;
    } else {
      out << gs << " 0 0" << endl;
    }
  }

  return 0;
}


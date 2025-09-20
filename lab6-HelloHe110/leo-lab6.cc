#include <iostream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nix-vector-helper.h"

using namespace ns3;
using namespace std;

// Satellite network setup
uint16_t port = 9;
NodeContainer satellites;
NodeContainer groundStations;
NetDeviceContainer utNet;
NetDeviceContainer islNet;
int Task = 2;
double duration = 100;


static void EchoMacTxRx(std::string context, const Ptr< const Packet > packet);
void SendPacket(int srcId, int dstId, std::vector<Ptr<PacketSink>>& sinks);
void connect();
LeoLatLong Vec2LatLong(Vector pos);
LeoLatLong id2LatLong(const NodeContainer &satellites, int id);

static void EchoMacTxRx(std::string context, const Ptr< const Packet > packet) {
    // Task 2.3: Complete this function
    // We only want to trace TCP packets with a data payload.
    Ptr<Packet> pCopy = packet->Copy();

    TcpHeader tcpHeader;
    
    if (pCopy->PeekHeader(tcpHeader)) {
        pCopy->RemoveHeader(tcpHeader); // Remove TCP header from the copy
        // If the remaining size is greater than 0, there is a data payload.
        if (pCopy->GetSize() == 570) {
            // This is a TCP packet with data. Let's trace it.
            std::string nodeIdStr = context.substr(context.find("NodeList/") + 9);
            nodeIdStr = nodeIdStr.substr(0, nodeIdStr.find("/"));
            uint32_t nodeId = std::stoi(nodeIdStr);

            Time now = Simulator::Now();
            std::string eventType = (context.find("MacTx") != std::string::npos) ? "MacTx" : "MacRx";
            
            std::cout << eventType << " at node: " << nodeId << ", now: " << now << std::endl;
        }
    }
} 

void SendPacket(int srcId, int dstId, std::vector<Ptr<PacketSink>>& sinks) {
    // Task 2.1: Complete this function
    // Task 2.1: Set MaxBytes to 512 & Task 3.1: Set MaxBytes to 0
    Ptr<Node> srcNode = NodeList::GetNode(srcId);
    Ptr<Node> dstNode = NodeList::GetNode(dstId);

    // Install PacketSink on destination
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(dstNode);
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(duration));
    // Keep track of sink for throughput calc
    sinks.push_back(StaticCast<PacketSink>(sinkApp.Get(0)));

    // Resolve IPv4 address of destination
    Ptr<Ipv4> ipv4 = dstNode->GetObject<Ipv4>();
    Ipv4Address dstAddr = ipv4->GetAddress(1, 0).GetLocal();

    // Configure BulkSend application
    BulkSendHelper sourceHelper("ns3::TcpSocketFactory", InetSocketAddress(dstAddr, port));
    uint32_t maxBytes = (Task == 2 ? 512 : 0);
    sourceHelper.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApp = sourceHelper.Install(srcNode);
    sourceApp.Start(Seconds(0.0));
    sourceApp.Stop(Seconds(duration));
}

void connect() {
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacTx", MakeCallback (&EchoMacTxRx));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacRx", MakeCallback (&EchoMacTxRx));
}

// Vec2LatLong function change xyz coordinate to latitude and longtitude
LeoLatLong Vec2LatLong(Vector pos){
    double r = sqrt (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
    double lat = asin (pos.z / r) * 180.0 / M_PI;
    double longit = atan2 (pos.y, pos.x) * 180 / M_PI;
    return LeoLatLong(lat, longit);
}

LeoLatLong id2LatLong(const NodeContainer &satellites, int id){
    Vector pos = satellites.Get(id)->GetObject<MobilityModel>()->GetPosition();
    return Vec2LatLong(pos);
}

NS_LOG_COMPONENT_DEFINE ("Lab6");

int main (int argc, char *argv[]) {
    CommandLine cmd;
    string constellation = "TelesatGateway";
    double duration = 100;
    string inputFile = "<Your Input File>";
    string outputFile;

    cmd.AddValue("in", "Input File", inputFile);
    cmd.AddValue("out", "Output File", outputFile);
    cmd.AddValue("Task", "2 or 3", Task);
    cmd.Parse (argc, argv);

    // Redirect cout if outputFile is specified
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::ofstream out;
    out.open (outputFile);
    if (out.is_open ()) {
        std::cout.rdbuf(out.rdbuf());
    }
    
    if(Task != 2 && Task != 3){
        cerr<<"Error: Task is not 2 or 3"<<endl;
        exit(1);
    }

    // Default setting
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(2.0)));

    // Satellite
    LeoOrbitNodeHelper orbit;
    satellites = orbit.Install ({ LeoOrbit (1200, 20, 6, 6) });

    // Ground station
    LeoGndNodeHelper ground;
    groundStations.Add(ground.Install(id2LatLong(satellites, 0), id2LatLong(satellites, 2)));  // node id=36, 37
    groundStations.Add(ground.Install(id2LatLong(satellites, 3), id2LatLong(satellites, 9)));  // node id=38, 39
    groundStations.Add(ground.Install(id2LatLong(satellites, 20), id2LatLong(satellites, 34))); // node id=40, 41

    // Set network
    LeoChannelHelper utCh;
    utCh.SetConstellation (constellation);
    utCh.SetGndDeviceAttribute("DataRate", StringValue("11kbps"));
    utNet = utCh.Install (satellites, groundStations);
    IslHelper islCh;
    islNet = islCh.Install (satellites);

    InternetStackHelper stack;
    // Set NVR as routing protocol 
    Ipv4NixVectorHelper nixRouting;
    nixRouting.SetPathFile(inputFile); // Implement the function
    stack.SetRoutingHelper(nixRouting);
    // Install internet stack on nodes
    stack.Install (satellites);
    stack.Install (groundStations);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.0.0", "255.255.0.0");
    ipv4.Assign (utNet);
    ipv4.SetBase ("10.2.0.0", "255.255.0.0");
    ipv4.Assign (islNet);

    // Task 2.2 & Task 3.2 : Call SendPacket()
    std::vector<Ptr<PacketSink>> sinks;
    if (Task == 2) {
        SendPacket(36, 38, sinks);
    }
    else if (Task == 3) {
        SendPacket(36, 38, sinks);
        SendPacket(37, 40, sinks);
        SendPacket(39, 41, sinks);
    }

    // Task 3.3 : Install sink applications for each destination is handled in SendPacket

    if(Task == 2) {
        // trace the packet in task2
        Simulator::Schedule(Seconds(1e-9), &connect);
    }
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();
    Simulator::Destroy ();

    // Task 3.4 : Calculate throughput
    if (Task == 3) {
        double totalBytes = 0;
        
        // Throughput for 36 -> 38
        double throughput1 = sinks[0]->GetTotalRx();
        std::cout << "36->38: " << throughput1 << std::endl;
        totalBytes += throughput1;

        // Throughput for 37 -> 40
        double throughput2 = sinks[1]->GetTotalRx();
        std::cout << "37->40: " << throughput2 << std::endl;
        totalBytes += throughput2;

        // Throughput for 39 -> 41
        double throughput3 = sinks[2]->GetTotalRx();
        std::cout << "39->41: " << throughput3 << std::endl;
        totalBytes += throughput3;

        std::cout << "Total throughput: " << totalBytes << std::endl;
    }

    out.close ();
    std::cout.rdbuf(coutbuf);

    return 0;
}

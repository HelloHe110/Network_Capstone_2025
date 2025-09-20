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
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include <iostream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"
#define SAT_Idx 0

using namespace ns3;

map<uint32_t, double> delay;
std::map<uint32_t, Time> sendTime;

static void EchoTxRx (std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket) {
    uint32_t seq = header.GetSequenceNumber().GetValue();
    Time now = Simulator::Now();

    // std::cout << Simulator::Now () << ":" << context << ":" << packet->GetUid() << ":" << socket->GetNode () << ":" << header.GetSequenceNumber () << std::endl;

    // 根據 Context 判斷是發送還是接收
    if (context.find("/Tx") != std::string::npos && sendTime[seq] == (Time)0) {
        sendTime[seq] = now;  // 記錄發送時間
    }

    if (context.find("/Rx") != std::string::npos) {
        if (sendTime.find(seq) != sendTime.end()) {
            Time delayTime = now - sendTime[seq];
            delay[seq] = delayTime.GetSeconds();  // 儲存計算後的端到端延遲
            // std::cout << "[Packet Info] Seq: " << seq << std::endl;
            // std::cout << "  - Send Time: " << sendTime[seq].GetSeconds() << " sec" << std::endl;
            // std::cout << "  - Receive Time: " << now.GetSeconds() << " sec" << std::endl;
            // std::cout << "  - End-to-End Delay: " << delay[seq] << " sec" << std::endl;
            // std::cout << "----------------------------------------" << std::endl;
        }
    }
}
static double readPathloss() {
    double pathLoss = 0.0;
    std::ifstream inFile("/home/user/ns-3.35/contrib/leo/examples/pathloss.txt");
    if (inFile.is_open()) {
        inFile >> pathLoss;
        inFile.close();
        // std::cout << "debug: pathLoss: " << pathLoss << std::endl;
    } else {
        printf("[!]Could not open pathloss.txt file\n");
    }
    return pathLoss;
}
void connect () {
    Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Tx", MakeCallback (&EchoTxRx));
    Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx", MakeCallback (&EchoTxRx));
}
void initial_position (const NodeContainer &satellites, int sz) {
    for (int i = 0; i < min((int)satellites.GetN(), sz); i++){
        // Get satellite position
        // TODO: get (x, y, z)
        Vector pos = satellites.Get(i)->GetObject<MobilityModel>()->GetPosition();
        // cout << "Satellite " << i << " (x, y, z) = (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
        
        // Convert position to latitude & longitude 
        double r = sqrt(pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
        double lat = asin(pos.z / r) * 180.0 / M_PI;
        double longit = atan2(pos.y, pos.x) * 180 / M_PI;
        cout << "Satellite " << i << " latitude = " << lat << ", longitude  = " << longit << endl;
    }
}

NS_LOG_COMPONENT_DEFINE ("LeoBulkSendTracingExample");

int main (int argc, char *argv[])
{

    CommandLine cmd;
    std::string orbitFile;
    std::string traceFile;
    // TODO: Set up topology - Rx (6.06692, 73.0213)
    // LeoLatLong source (6.06692, 73.0213);
    // LeoLatLong destination (6.06692, 73.0213);
    LeoLatLong source (20, 0);
    LeoLatLong destination (20, 0);
    // LeoLatLong source (-16.0634, 142.29);
    // LeoLatLong destination (-16.0634, 142.29);
    std::string islRate = "2Gbps";
    std::string constellation = "TelesatGateway";
    uint16_t port = 9;
    uint32_t latGws = 20;
    uint32_t lonGws = 20;
    double duration = 100;
    bool islEnabled = false;
    bool pcap = false;
    uint64_t ttlThresh = 0;
    std::string routingProto = "aodv";

    cmd.AddValue("orbitFile", "CSV file with orbit parameters", orbitFile);
    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("precision", "ns3::LeoCircularOrbitMobilityModel::Precision");
    cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
    cmd.AddValue("source", "Traffic source", source);
    cmd.AddValue("destination", "Traffic destination", destination);
    cmd.AddValue("islRate", "ns3::MockNetDevice::DataRate");
    cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
    cmd.AddValue("routing", "Routing protocol", routingProto);
    cmd.AddValue("islEnabled", "Enable inter-satellite links", islEnabled);
    cmd.AddValue("latGws", "Latitudal rows of gateways", latGws);
    cmd.AddValue("lonGws", "Longitudinal rows of gateways", lonGws);
    cmd.AddValue("ttlThresh", "TTL threshold", ttlThresh);
    cmd.AddValue("destOnly", "ns3::aodv::RoutingProtocol::DestinationOnly");
    cmd.AddValue("routeTimeout", "ns3::aodv::RoutingProtocol::ActiveRouteTimeout");
    cmd.AddValue("pcap", "Enable packet capture", pcap);
    cmd.Parse (argc, argv);

    std::streambuf *coutbuf = std::cout.rdbuf();
    // redirect cout if traceFile
    std::ofstream out;
    out.open (traceFile);
    if (out.is_open ()) std::cout.rdbuf(out.rdbuf()); 

    LeoOrbitNodeHelper orbit;
    NodeContainer satellites;
    if (!orbitFile.empty()) satellites = orbit.Install (orbitFile);
    else satellites = orbit.Install ({ LeoOrbit (1200, 20, 5, 5) });

    LeoGndNodeHelper ground;
    NodeContainer users = ground.Install (source, destination);

    LeoChannelHelper utCh;
    utCh.SetConstellation (constellation);

    // HERE
    // Task 4.1: Set up transmission configuration
    // utCh.SetGndDeviceAttribute("DataRate", StringValue("8kbps"));
    
    NetDeviceContainer utNet = utCh.Install (satellites, users);

    initial_position(satellites, 5);
    // TODO: get (x, y, z)
    Vector pos_tx, pos_sat;
    pos_tx = utNet.Get(25)->GetNode()->GetObject<MobilityModel>()->GetPosition();
    pos_sat = utNet.Get(SAT_Idx)->GetNode()->GetObject<MobilityModel>()->GetPosition();
    // TODO: write into the txt file
    auto write_txt = [](Vector pos_tx, Vector pos_sat) {
        std::ofstream outfile("contrib/leo/examples/xyz_positions.txt");
        
        if (outfile.is_open()) {
            outfile << "" << std::to_string(pos_tx.x) << " " << std::to_string(pos_tx.y) << " " << std::to_string(pos_tx.z) << "\n";
            outfile << "" << std::to_string(pos_sat.x) << " " << std::to_string(pos_sat.y) << " " << std::to_string(pos_sat.z) << "\n";
            outfile.close();
            std::cout << "Output written to xyz_positions.txt" << std::endl;
        } else {
            std::cout << "Unable to open file xyz_positions.txt" << std::endl;
        }
    };
    write_txt(pos_tx, pos_sat);

    // Task 4.2-4: Calculate received power, SNR, and Shannon capacity
    // Ptr<LeoPropagationLossModel> propagationLossModel = CreateObject<LeoPropagationLossModel>();
    Ptr<NetDevice> dev = utNet.Get(SAT_Idx);
    Ptr<LeoMockNetDevice> mockDev = DynamicCast<LeoMockNetDevice>(dev);
    Ptr<LeoMockChannel> channel = DynamicCast<LeoMockChannel>(mockDev->GetChannel());
    Ptr<LeoPropagationLossModel> propagationLossModel = DynamicCast<LeoPropagationLossModel>(channel->GetPropagationLoss());

    if (propagationLossModel != nullptr) {
        // Get configured values from the device
        double txPowerDbm = 105.9; // Tx power
        double noisePowerDbm = -110.0; // Noise power
        double bandwidthHz = 2e6; // 2MHz in Hz
        
        
        Ptr<MobilityModel> txMobility = users.Get(0)->GetObject<MobilityModel>();
        Ptr<MobilityModel> rxMobility = satellites.Get(SAT_Idx)->GetObject<MobilityModel>();
        
        double rxPowerDbm = propagationLossModel->DoCalcRxPower(txPowerDbm, txMobility, rxMobility);
        double euclidean_dis = sqrt((pos_tx.x - pos_sat.x)*(pos_tx.x - pos_sat.x) + (pos_tx.y - pos_sat.y)*(pos_tx.y - pos_sat.y) + (pos_tx.z - pos_sat.z)*(pos_tx.z - pos_sat.z));
        
        double snrDb = rxPowerDbm - noisePowerDbm;
        double snrRatio = std::pow(10.0, snrDb / 10.0);
        double shannonCapacity = bandwidthHz * std::log2(1 + snrRatio);

        // HERE
        // printf("\n====== w/o bf - Results ======\n");
        printf("\n====== w/  bf - Results ======\n");
        printf("Q1: Euclidean Dis: %.6f m\n", euclidean_dis);
        printf("Q2: PathLoss: %.6f dBm\n", readPathloss());
        printf("Q3: RxPower: %.6f dBm\n", rxPowerDbm);
        printf("Q4: SNR: %.6f dB\n", snrDb);
        printf("Q5: DataRate: %.6f Mbps\n", shannonCapacity / 1e6);

        // HERE
        // Task 5.1: Update link data rate with Shannon capacity
        std::stringstream oss;
        oss << shannonCapacity / 1e6 << "Mbps";  // Convert to Mbps for DataRate string
        utNet.Get(25)->GetObject<MockNetDevice>()->SetDataRate(DataRate(oss.str()));  // Update user[0]'s data rate
        utNet.Get(SAT_Idx)->GetObject<MockNetDevice>()->SetDataRate(DataRate("1Gbps"));     // Set satellite[1]'s data rate to 1Gbps

        // std::cout << "Updated Data Rate: " << oss.str() << std::endl;
        // std::cout << "Satellite[" << SAT_Idx << "]" << "Data Rate: 1Gbps" << std::endl;
    } else { std::cout << "Failed to get propagation loss model" << std::endl; }

    InternetStackHelper stack;
    AodvHelper aodv;
    aodv.Set ("EnableHello", BooleanValue (false));
    //aodv.Set ("HelloInterval", TimeValue (Seconds (10)));
    if (ttlThresh != 0) {
        aodv.Set ("TtlThreshold", UintegerValue (ttlThresh));
        aodv.Set ("NetDiameter", UintegerValue (2*ttlThresh));
    }
    stack.SetRoutingHelper (aodv);

    // Install internet stack on nodes
    stack.Install (satellites);
    stack.Install (users);

    Ipv4AddressHelper ipv4;

    ipv4.SetBase ("10.1.0.0", "255.255.0.0");
    ipv4.Assign (utNet);

    if (islEnabled)  {
        // std::cerr << "ISL enabled" << std::endl;
        IslHelper islCh;
        NetDeviceContainer islNet = islCh.Install (satellites);
        ipv4.SetBase ("10.2.0.0", "255.255.0.0");
        ipv4.Assign (islNet);
    }

    Ipv4Address remote = users.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
    BulkSendHelper sender ("ns3::TcpSocketFactory", InetSocketAddress (remote, port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    sender.SetAttribute ("MaxBytes", UintegerValue (1024));
    sender.SetAttribute ("SendSize", UintegerValue (512));
    ApplicationContainer sourceApps = sender.Install (users.Get (0));
    sourceApps.Start (Seconds (0.0));

    //
    // Create a PacketSinkApplication and install it on node 1
    //
    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sink.Install (users.Get (1));
    sinkApps.Start (Seconds (0.0));

    // Fix segmentation fault
    Simulator::Schedule(Seconds(1e-7), &connect);

    //
    // Set up tracing if enabled
    //
    if (pcap) {
        AsciiTraceHelper ascii;
        utCh.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
        utCh.EnablePcapAll ("tcp-bulk-send", false);
    }

    // std::cerr << "LOCAL =" << users.Get (0)->GetId () << std::endl;
    // std::cerr << "REMOTE=" << users.Get (1)->GetId () << ",addr=" << Ipv4Address::ConvertFrom (remote) << std::endl;

    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
    // std::cout << users.Get (0)->GetId () << ":" << users.Get (1)->GetId () << ": " << sink1->GetTotalRx () << std::endl;

    // Task 5.2: Output end-to-end delay
    // The delay is already being calculated in the EchoTxRx callback function
    // We just need to calculate and display the average delay at the end
    double avg_delay = 0;
    for (auto &[key, t] : delay) avg_delay += t; 
    if (!delay.empty()) avg_delay /= delay.size(); 
    printf("Packet average end-to-end delay is %.6f s\n", avg_delay);
    printf("======== END ========\n\n");

    out.close ();
    std::cout.rdbuf(coutbuf);

    return 0;
}

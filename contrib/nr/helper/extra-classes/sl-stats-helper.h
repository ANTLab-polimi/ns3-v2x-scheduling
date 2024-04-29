/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SL_STATS_HELPER_H
#define SL_STATS_HELPER_H

// #include "ns3/elephant-flow.h"
#include <ns3/lte-ue-rrc.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/netanim-module.h>
#include "ns3/eps-bearer-tag.h"
//#include <ns3/node-container.h>
#include "ns3/internet-module.h" 
#include "ns3/applications-module.h"
 #include "ns3/traffic-control-module.h"
//#include <ns3/ipv4-address-helper.h>
//#include "ns3/internet-module.h"
//#include "ns3/applications-module.h"
#include <ns3/nr-gnb-phy.h>
#include <iostream>
#include <string>
#include <ns3/antenna-module.h>
#include <ns3/nstime.h>

#include "sl-flow-monitor-output-stats.h"
#include "sl-power-output-stats.h"
#include "sl-rb-output-stats.h"
#include "sl-sinr-output-stats.h" 
#include "sl-slot-output-stats.h"
#include "sl-slot-ctrl-output-stats.h"
#include "sl-slot-stats.h"
#include "sl-downlink-tb-size-stats.h"
#include "sl-uplink-tb-size-stats.h"
#include "sl-ctrl-ul-dci-stats.h"
#include "sl-mac-sr-stats.h"
#include "sl-packet-trace-stats.h"
#include "sl-mac-rlc-buffer-status.h"
#include "sl-ue-bsr-stats.h"
#include "sl-ue-pss-received-stats.h"
#include "nr-dl-scheduling-stats.h"
#include "sl-ctrl-msgs-stats.h"
#include "sl-mobility-stats.h"
#include <functional>

#include "ns3/ue-mac-pscch-tx-output-stats.h"
#include "ns3/ue-mac-pssch-tx-output-stats.h"
#include "ns3/ue-phy-pscch-rx-output-stats.h"
#include "ns3/ue-phy-pssch-rx-output-stats.h"
#include "ns3/ue-to-ue-pkt-txrx-output-stats.h"
#include "ns3/ue-v2x-scheduling-xapp-stats.h"

// Stats necessary to extract the information
#include "ns3/nr-spectrum-value-helper.h"


namespace ns3 {

typedef std::map< uint32_t, std::vector<Ipv4Address> > NodeAddressesMap;
typedef std::map< std::string, NodeAddressesMap > ContainerAddresses;
typedef int (*fptr)();

class SlStatsHelper : public Object{

public:

	SlStatsHelper(void);

	virtual ~SlStatsHelper(void);

	/**
	   * \brief Get the type ID.
	   * \return the object TypeId
	   */
	static TypeId GetTypeId (void);
//	std::string m_statsCheck = "/home/ugjeci/Desktop/bbr-results/StatsCheck/";

private:
//	"/home/" + "%s" + "/Desktop/bbr-results"
	std::string m_dirPath, m_cwndDir, m_pcapDir, m_statsCheck;
	// data to be used by the AI algorithm to check for EF


	void createDir(std::string dirPath);

public:

	// getter & setter the parameters functions
	void SetCongestionsWindowDirectory(std::string value);

	void SetPcapDirectory (std::string value);

	void SetResultsDirectory(std::string value);

	void SetStatsCheckDirectory(std::string value);

	void CwndChange (uint32_t oldCwnd, uint32_t newCwnd);

	Ptr<NrRadioEnvironmentMapHelper> createRem();

	static void 
	BulkApplicationTxPacketSingle(Ptr<const Packet> p);

	static void CourseChange (SlMobilityStats *stats, Ptr<const MobilityModel> mobility);

	static void
	ReportSinrNr (SlSinrOutputStats *stats, uint16_t cellId, uint16_t rnti,
				double power, double avgSinr, uint16_t bwpId);
	static void
	ReportPowerNr (SlPowerOutputStats *stats, const SfnSf & sfnSf,
				 Ptr<const SpectrumValue> txPsd, const Time &t, uint16_t rnti, uint64_t imsi,
				 uint16_t bwpId, const uint16_t cellId);
	static void
	ReportSlotStatsNr (SlSlotOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
					 uint32_t usedReg, uint32_t usedSym,
					 uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
					 uint16_t cellId);

	static void
	ReportSlotCtrlStatsNr (SlSlotCtrlOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
						 uint32_t usedReg, uint32_t usedSym,
						 uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
						 uint16_t cellId);

	static void
	ReportSlPhySlotStats(SlPhySlotStats *phySlotStats, const SfnSf &sfnSf,
						std::deque<ns3::VarTtiAllocInfo> queue, uint32_t availRb, uint32_t NumRbPerRbg,  uint16_t bwpId, uint16_t cellId);


	static void
	ReportRbStatsNr (SlRbOutputStats *stats, const SfnSf &sfnSf, uint8_t sym,
								const std::vector<int> &rbUsed, uint16_t bwpId,
								uint16_t cellId);
	static void
	ReportGnbRxDataNr (SlPowerOutputStats *gnbRxDataStats, const SfnSf &sfnSf,
								  Ptr<const SpectrumValue> rxPsd, const Time &t, uint16_t bwpId,
								  uint16_t cellId);

	void
	ReportFlowRatesNr (SlFlowMonitorOutputStats *flowRatesStats,
						const Ptr<FlowMonitor> &monitor, FlowMonitorHelper &flowmonHelper,
						const std::string &filename);

	static void
	ReportULTbSize (SlUplinkTbSizeStats *ulTbSizeStats, uint64_t imsi, uint64_t tbsize);

	static void
	ReportDLTbSize (SlDownlinkTbSizeStats *dlTbSizeStats, uint64_t imsi, uint64_t tbsize);

	// mac traces
	static void
	ReportMacSR(SlMacSRStats *macSRStats, uint8_t bwdId, uint16_t rnti);

	static void 
	ReportUeMacBsr(SlMacUeBsrStats *macUeBsrStats, uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint16_t rnti, 
	uint8_t bsrid, uint8_t lcid, uint32_t bufferSize);

	static void
	ReportMacBsr(SlMacBsrStats *macSRStats, SfnSf sfnsf, uint16_t cellId, NrMacSchedSapProvider::SchedDlRlcBufferReqParameters schedParams);


	static void
	RxPacketTraceUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params);

	static void 
	ReportCtrlUlDci(SlCtrlUlDciStats *ctrlUlDciStats, uint16_t cellId, const std::shared_ptr<DciInfoElementTdma> &dci,const SfnSf &sfnsf);

	// packet trace
	static void
	ReportPacketTrace(SlPacketTraceStats *packetTraceStats, std::string direction, RxPacketTraceParams packetTrace);

	// the ue received pss
	static void
	ReportUeReceivedPss(SlUePssReceivedStats *uePssReceivedStats, const SfnSf &sfnsf, std::vector<uint16_t> userIds, uint64_t imsi, uint16_t generatingCellId, double power, uint16_t nRb, double power_rsrq, uint16_t nRb_rsrq);

	static void 
	ReportCtrlMessages(SlCtrlMsgsStats *ctrlMsgsStats, std::string layer, std::string entity, SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg);


	// sl 
	static void
	PrintGnuplottableUeListToFile(std::string filename);

	/*
	* Global methods and variable to hook trace sources from different layers of
	* the protocol stack.
	*/

	static void 
	NotifyxAppScheduling(UeV2XSchedulingXApp* v2xSchedStats, Ptr<NrGnbPhy> gnbPhyPtr, uint64_t ueId, ns3::NrSlGrantInfo nrSlGrantInfo, std::string plmnId);


	/**
	 * \brief Method to listen the trace SlPscchScheduling of NrUeMac, which gets
	 *        triggered upon the transmission of SCI format 1-A from UE MAC.
	 *
	 * \param pscchStats Pointer to the UeMacPscchTxOutputStats class,
	 *        which is responsible to write the trace source parameters to a database.
	 * \param pscchStatsParams Parameters of the trace source.
	 */
	static void 
	NotifySlPscchScheduling (UeMacPscchTxOutputStats *pscchStats, const SlPscchUeMacStatParameters pscchStatsParams);

	/**
	 * \brief Method to listen the trace SlPsschScheduling of NrUeMac, which gets
	 *        triggered upon the transmission of SCI format 2-A and data from UE MAC.
	 *
	 * \param psschStats Pointer to the UeMacPsschTxOutputStats class,
	 *        which is responsible to write the trace source parameters to a database.
	 * \param psschStatsParams Parameters of the trace source.
	 */
	static void 
	NotifySlPsschScheduling (UeMacPsschTxOutputStats *psschStats, const SlPsschUeMacStatParameters psschStatsParams);

	/**
	 * \brief Method to listen the trace RxPscchTraceUe of NrSpectrumPhy, which gets
	 *        triggered upon the reception of SCI format 1-A.
	 *
	 * \param pscchStats Pointer to the UePhyPscchRxOutputStats class,
	 *        which is responsible to write the trace source parameters to a database.
	 * \param pscchStatsParams Parameters of the trace source.
	 */
	static void 
	NotifySlPscchRx (UePhyPscchRxOutputStats *pscchStats, const SlRxCtrlPacketTraceParams pscchStatsParams);

	/**
	 * \brief Method to listen the trace RxPsschTraceUe of NrSpectrumPhy, which gets
	 *        triggered upon the reception of SCI format 2-A and data.
	 *
	 * \param psschStats Pointer to the UePhyPsschRxOutputStats class,
	 *        which is responsible to write the trace source parameters to a database.
	 * \param psschStatsParams Parameters of the trace source.
	 */
	static void 
	NotifySlPsschRx (UePhyPsschRxOutputStats *psschStats, const SlRxDataPacketTraceParams psschStatsParams);

	/**
	 * \brief Method to listen the application level traces of type TxWithAddresses
	 *        and RxWithAddresses.
	 * \param stats Pointer to the UeToUePktTxRxOutputStats class,
	 *        which is responsible to write the trace source parameters to a database. *
	 * \param node The pointer to the TX or RX node
	 * \param localAddrs The local IPV4 address of the node
	 * \param txRx The string indicating the type of node, i.e., TX or RX
	 * \param p The packet
	 * \param srcAddrs The source address from the trace
	 * \param dstAddrs The destination address from the trace
	 * \param seqTsSizeHeader The SeqTsSizeHeader
	 */
	static void
	UePacketTraceDb (UeToUePktTxRxOutputStats *stats, Ptr<Node> node, const Address &localAddrs,
					std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
					const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader);
};


void RxRlcPDUSl (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay);

void RxPdcpPDUSl (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay);

/**
 * Function that connects PDCP and RLC traces to the corresponding trace sources.
 */
void ConnectPdcpRlcTracesSl ();

OperationBandInfo configureBandwidth(Ptr<NrHelper> & nrHelper, double centralFrequencyBand, double bandwidthBand);

/**
 * Function that connects UL PDCP and RLC traces to the corresponding trace sources.
 */
void ConnectUlPdcpRlcTracesSl ();

void ReportRlcPacketDrop(Ptr<OutputStreamWrapper> streamRlc, Ptr<const Packet> p);

/*
 * set default gateware of the nodes
 * */
// NrPointToPointEpcHelper
// void setDefaultGateway(NodeContainer nodes, Ptr<NrNewPointToPointEpcHelper> epcHelper,
// 		Ipv4StaticRoutingHelper ipv4RoutingHelper);

/*
 * set antenna attributes
 * */
void setUEAntennaAttributes(Ptr<NrHelper> nrHelper);
void setGNBAntennaAttributes(Ptr<NrHelper> nrHelper);

/*
 * Set Phy Attributes
 * */
void setGNBPhyAttributes(Ptr<NrHelper> nrHelper, Ptr<NetDevice> gnbNode, const uint16_t numerologyBwp, const double totalTxPower);
void setUePhyAttributes(Ptr<NrHelper> nrHelper, Ptr<NetDevice> ueNode, const double totalTxPower, const double noiseFigure, const bool UlPowerControl);

/*
 * Set Amc Attributes
 * */
void setGNBAmcAttributes(Ptr<NrHelper> nrHelper);

void SendPacket (Ptr<NetDevice> device, Address& addr, uint32_t packetSize);

// print the nested map of addresses
void printNestedMap(ContainerAddresses contAddrStruct);

/*
 * Update config for the container
 * */
template <typename T>
void updateContainerConfig(NetDeviceContainer container){
	for (auto it = container.Begin (); it != container.End (); ++it){
		Ptr<T> netDev = DynamicCast<T> (*it);
		if(netDev){
			netDev->UpdateConfig ();
		}
	}
}
//void updateContainerConfig(NetDeviceContainer container);

// get Node Id by address in structure
int getNodeIdByAddress(ContainerAddresses contAddrStruct, Ipv4Address nodeAddr);

void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd);

void StartSinglePacketFlow (Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort, const uint32_t elephantFlowSize);

void initializeTxBuffer();

/*
 * Create the header of csv file
 * */
void createFileHeader(std::string fileName);

void installServerSink(Ptr<Node> node, Ipv4Address ipv4, uint16_t port, std::string socketFactory, ApplicationContainer apps);

void installPacketSink (Ptr<Node> node, uint16_t port, std::string socketFactory, ApplicationContainer apps);

void connectTracesToPrintStatement(bool isUl);

/*
 * Create P2P Helper to connect pgw with the remote host
 * */
PointToPointHelper createP2PHElper(std::string dataRate, uint64_t delayMicroseconds);

/*
 * Configure TCP Traffic
 * */
void configureTCPTraffic(NodeContainer clientTcpNodes, ApplicationContainer clientApps, NodeContainer serverTcpNodes, Ipv4InterfaceContainer  serverTcpIps, uint16_t tcpPort);

/*
 * Configure UDP traffic
 * */
void configureUDPTraffic(NodeContainer clientNodes, ApplicationContainer* clientApps, NodeContainer serverNodes, Ipv4InterfaceContainer  serverIps, uint16_t udpPort);

void configureSingleUDPTraffic(NodeContainer clientNodes, ApplicationContainer* clientApps, NodeContainer serverNodes, Ipv4InterfaceContainer  serverIps, uint16_t udpPort);

std::string getStrFromInt(int number);

}

#endif /* ELEPHANT_FLOW_HELPER_H */


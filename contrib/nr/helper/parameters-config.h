/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <string>
#include <ostream>
#include <ns3/object.h>
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include <ns3/lte-enb-net-device.h>

#ifndef _PARAMETERS_CONFIG_H
#define _PARAMETERS_CONFIG_H

namespace ns3 {

typedef struct BulkSenderStruct
{
	Ptr<Socket> socket;
	Ptr<Ipv4> servertcpIpv4;
	uint32_t *pointerToSize;
	Ptr<OutputStreamWrapper> stream;
	Ptr<BulkSendApplication> bulkSenderApplication;
} BulkSenderStruct_t;

struct Params
{
	std::string simulator = "5GLENA";
	// open ai variables
	uint32_t openGymPort = 5555;

	std::string scenario = "UMa";

	// 7 gnb by default to create the hexagonal
	uint16_t gNbNum = 3;
	uint16_t numSectorsGnb = 1;
	uint16_t numOuterRings = 1;
	uint16_t ueNumPergNb = 90;
	uint16_t ueNumPerSector = 30;
	uint16_t numFlowsUe = 3;

	// modified
	std::string activeElephantFlowStartTileFilename = "activeElephantFlowUsersStartTimeFilename.txt";
	std::string elephantFlowUsersIndexFilename = "elephantFlowUsersFilename.txt";
	std::string gNbPositionFilename = "gnbPosition.txt";
	std::string positionRegularUsersFilename = "regUsersPosition.txt";
	std::string positionElephantFlowUsersFilename = "elephantFlowUsersPosition.txt";
	// end modification

	uint8_t numBands = 1;
	double centralFrequencyBand = 28e9;
	double bandwidthBand = 200e6;

	bool contiguousCc = true;
	uint16_t numerology = 3;

	//non-contiguous case
	double centralFrequencyCc0 = 28e9;
	double centralFrequencyCc1 = 29e9;
	double bandwidthCc0 = 200e6;
	double bandwidthCc1 = 100e6;
	uint16_t numerologyCc0Bwp0 = 3;
	uint16_t numerologyCc0Bwp1 = 4;
	uint16_t numerologyCc1Bwp0 = 3;


	std::string pattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
	double totalTxPower = 8;
	bool cellScan = true;
	double beamSearchAngleStep = 10.0;

	std::string burstIntensityFilename = "burstIntensity.txt";
	std::string meanBurstArrivalTimeFilename = "meanBurstArrivalTime.txt";
	std::string meanBurstDurationFilename = "meanBurstDuration.txt";
	std::string hurstFilename = "hurst.txt";

	uint32_t udpPacketSizeUll = 100;
	uint32_t udpPacketSizeBe = 1252;
	uint32_t lambdaUll = 10000;
	uint32_t lambdaBe = 1000;

	bool logging = false;

	bool disableDl = false;
	bool disableUl = true;

	
	uint16_t numActiveRegularUsers = 5;

	// original
	uint16_t numElephantFlowUsers = 7;
	uint16_t numRegularUsers = 6;
	uint16_t numActiveElephantFlowUsers = 1;
	uint32_t gNbDistance = 500;
	// std::string simTag = "act_reg_10_tot_50_active_efs_1_tot_efs_1_no_hand";
	// std::string simTag = "act_reg_6_tot_6_active_efs_1_tot_efs_1_no_hand";
	
	// std::string outputDir = "/storage/franci/generic/test/";
	bool handoverSimulationHasHandover = false;
	double handoverTime = 0.11;
	uint32_t handoverSrcGnbIndex = 0;
	uint32_t handoverDstGnbIndex = 0;
	uint32_t handoverNodeIndex = 0;
	bool handoverNodeIsEf = false;
	double simTime = 0.5;
	// end modification

	std::string cbr = "10Mb/s";
	// sector where we can simulate the scenario by coping the parameters

	uint32_t txBufferSize = 1024*300;
	std::string simTag = "act_reg_20_tot_20_active_efs_1_tot_efs_7";
	std::string outputDir = "/storage/franci/handover/";
	std::string ltePlmnId = "111";
	std::string ricE2TermIpAddress = "10.0.2.10";
	uint16_t ricE2TermPortNumber = 36422;
	uint16_t e2startingPort = 38470;
	bool isXappEnabled = false;
	double ueDiscCenterXPosition = 0;
	double ueDiscCenterYPosition = 0;
	// bool handoverSimulationHasHandover = true;
	// double handoverTime = 0.9001;
	// uint32_t handoverSrcGnbIndex = 0;
	// uint32_t handoverDstGnbIndex = 2;
	// uint32_t handoverNodeIndex = 1;
	// bool handoverNodeIsEf = true;
	// double simTime = 2.20;
	// end copied parameters
	
	double udpAppStartTime = 0.4; //seconds

	bool Validate (void) const;
};

//extern void ParametersConfig (const Params &params);

class ParametersConfig : public Object{

	/**
	* create an Elephant-flow instance for use finding elephant flows
	*
	*/
//	ParametersConfig ();
public:
	ParametersConfig (void);

	static void DisableTraces();

	static void EnableTraces();

	static void CreateTracesDir(Params params);

	// static std::vector<uint32_t> GetEfsIndex(Params params);

	// static std::vector<double> GetMeanBurstArrivalTime(Params params);
	// static std::vector<double> GetMeanBurstDuration(Params params);

	// static std::vector<double> GetMeanBurstIntensity(Params params);

	// static std::vector<double> GetHurst(Params params);

	// static void 
	// BulkApplicationTxPacketSingle(Ptr<const Packet> p);

	// static void 
	// BulkApplicationTxPacket(BulkSenderStruct* bulkSenderStruct, Ptr<const Packet> packet);

	// static void 
	
	// PacketSinkRxPacket(Ptr<OutputStreamWrapper> stream, Ptr<ns3::Node> node, Ptr<Ipv4> ipv4, 
	// 	Ptr<const Packet> p, const ns3::Address & from, const ns3::Address & to);

	// static void 
	// BulkApplicationTxPacketLteCoordinator(BulkSenderStruct* bulkSenderStruct, Ptr<LteEnbNetDevice> reportingEnbDevice, Ptr<const Packet> packet);

	// static void RxUdpVideo (Ptr<OutputStreamWrapper> stream, Ptr<ns3::Node> node, Ptr<Ipv4> ipv4, Ptr<LteEnbNetDevice> reportingEnbDevice,
	// 	Ptr<const Packet> p, const ns3::Address & from, const ns3::Address & to);
	
	// static void 
	// RxFrameUdpVideo (Ptr<OutputStreamWrapper> stream, Ptr<ns3::Node> node, Ptr<Ipv4> ipv4, Ptr<LteEnbNetDevice> reportingEnbDevice,
	// 	uint32_t frameid, uint32_t frameSize, uint32_t ipAddress, uint16_t frameLevel);

	
	/**
	* Destructor
	*/
	virtual ~ParametersConfig ();
	/**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

};



}
#endif /* ELEPHANT_FLOW_H */


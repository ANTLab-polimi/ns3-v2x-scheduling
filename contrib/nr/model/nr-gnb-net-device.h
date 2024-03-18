/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef NR_ENB_NET_DEVICE_H
#define NR_ENB_NET_DEVICE_H

#include "nr-net-device.h"
#include <ns3/traced-callback.h>
#include <ns3/oran-interface.h>
#include <ns3/nr-sl-phy-mac-common.h>

#include "nr-sl-sci-f1a-header.h"
#include "nr-sl-mac-pdu-tag.h"
#include <vector>
#include <map> 

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class NrGnbPhy;
class NrGnbMac;
class LteEnbRrc;
class BandwidthPartGnb;
class LteEnbComponentCarrierManager;
class BwpManagerGnb;
class NrMacScheduler;

/**
 * \ingroup gnb
 * \brief The NrGnbNetDevice class
 *
 * This class represent the GNB NetDevice.
 */
class NrGnbNetDevice : public NrNetDevice
{
public:
  static TypeId GetTypeId (void);

  NrGnbNetDevice ();

  virtual ~NrGnbNetDevice (void);

  Ptr<NrMacScheduler> GetScheduler (uint8_t index) const;

  Ptr<NrGnbMac> GetMac (uint8_t index) const;

  Ptr<NrGnbPhy> GetPhy (uint8_t index) const;

  Ptr<BwpManagerGnb> GetBwpManager () const;

  uint16_t GetBwpId (uint8_t index) const;

  /**
   * \return the cell id
   */
  uint16_t GetCellId () const;

  /**
   * \return the cell ids belonging to this gNB
   */
  std::vector<uint16_t> GetCellIds () const;

  /**
   * \brief Set this gnb cell id
   * \param cellId the cell id
   */
  void SetCellId (uint16_t cellId);

  uint16_t GetEarfcn (uint8_t index) const;

  void SetRrc (Ptr<LteEnbRrc> rrc);

  Ptr<LteEnbRrc> GetRrc (void);

  void SetCcMap (const std::map<uint8_t, Ptr<BandwidthPartGnb> > &ccm);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

  /**
   * \brief The gNB received a CTRL message list.
   *
   * The gNB should divide the messages to the BWP they pertain to.
   *
   * \param msgList Message list
   * \param sourceBwpId BWP Id from which the list originated
   */
  void RouteIngoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList, uint8_t sourceBwpId);

  /**
   * \brief Route the outgoing messages to the right BWP
   * \param msgList the list of messages
   * \param sourceBwpId the source bwp of the messages
   */
  void RouteOutgoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList, uint8_t sourceBwpId);

  /**
   * \brief Update the RRC config. Must be called only once.
   */
  void UpdateConfig ();

  // modified
  // public 
  void SetE2Termination (Ptr<E2Termination> e2term);

  Ptr<E2Termination> GetE2Termination() const;

  // sci reception from the sl
  static void SciReceptionCallback (Ptr<NrGnbNetDevice> gnbNetDevicePtr, //std::string path,
                                  uint64_t imsi, uint16_t rnti,
                                 NrSlSciF1aHeader sciF1Header, NrSlMacPduTag sciTag,
                                 double rsrpdBm, double x, double y);


  static void PacketDelaysInBufferCallback (Ptr<NrGnbNetDevice> gnbNetDevicePtr, //std::string path,
                                    uint16_t rnti, std::map<uint16_t, PacketDelayStruct> packetDelaysInBuffer, 
                                    double xPosition, double yPosition);
  // end modification 

protected:
  virtual void DoInitialize (void);

  virtual void DoDispose (void);
  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

private:
  Ptr<LteEnbRrc> m_rrc;

  uint16_t m_cellId; //!< Cell ID. Set by the helper.

  std::map<uint8_t, Ptr<BandwidthPartGnb> > m_ccMap; /**< ComponentCarrier map */

  Ptr<LteEnbComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager of this eNb

  // modified
  Ptr<E2Termination> m_e2term;
  std::string m_ltePlmnId;
  bool m_forceE2FileLogging; //< if true log PMs to files
  bool m_isReportingEnabled;
  std::string m_cuUpFileName;
  std::string m_cuCpFileName;
  std::string m_tracesPath;
  std::string m_ricControlReceivedFilename;
  bool m_reducedPmValues;

  bool m_sendCuUp;
  bool m_sendCuCp;
  bool m_sendDu;
  double m_e2Periodicity;

  std::string m_controlFilename;

  void KpmSubscriptionCallback (E2AP_PDU_t* sub_req_pdu);
  void ControlMessageReceivedCallback (E2AP_PDU_t* sub_req_pdu);

  void test_buffer_msg();

  void PauseSimulation();

  void SendMessage(E2Termination::RicSubscriptionRequest_rval_s params, uint16_t generatingCellId,
                        Ptr<KpmIndicationHeader> header,
                          Ptr<KpmIndicationMessage> cuCpMsg);

  void SendSchedulerDataToTrace(uint64_t ueId, NrSlGrantInfo nrSlGrantInfo, std::string plmnId, bool isLastReport);

  TracedCallback<uint64_t, NrSlGrantInfo, std::string> m_e2V2XSchedulingTrace; 

  void BuildAndSendReportMessage(E2Termination::RicSubscriptionRequest_rval_s params);

  Ptr<KpmIndicationHeader> BuildRicIndicationHeader(std::string plmId, std::string gnbId, uint16_t nrCellId);
  // Ptr<KpmIndicationMessage> BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, size_t numOfReports);
  // Ptr<KpmIndicationMessage> BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId,
  //                                                             uint32_t numSingleReports, 
  //                                       std::map<uint64_t, std::vector<E2SciData>>::iterator begin,
  //                                       std::map<uint64_t, std::vector<E2SciData>>::iterator end, size_t numOfReports);
  Ptr<KpmIndicationMessage> BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId,
                                                              uint32_t numSingleReports, 
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itBegin,
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itEnd, 
                                        size_t numOfReports);
  Ptr<KpmIndicationMessage> BuildMillicarRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, size_t numOfReports);

  Ptr<KpmIndicationMessage> BuildRicIndicationMessageCuUp(std::string plmId);
  Ptr<KpmIndicationMessage> BuildRicIndicationMessageCuCp(std::string plmId);

  void SciE2Relay(uint64_t imsi, uint16_t rnti,
                                 NrSlSciF1aHeader sciF1Header, NrSlMacPduTag sciTag,
                                 double rsrpdBm, double x, double y);

  // modified
  void PacketDelaysInBuffer(uint16_t rnti, std::map<uint16_t, 
                                        PacketDelayStruct> packetDelaysInBuffer,
                                        double xPosition, double yPosition);
  
  struct AggregatePacketDelayReportStruct
  {
    
    uint32_t lowerIntervalLimit;
    uint32_t upperIntervalLimit;
    uint32_t numPackets;
  };

  struct BufferSize{
    uint32_t num_packets = 0; ///< RNTI
    uint32_t buffer_size = 0; ///< DRBID

    BufferSize(uint32_t num_packets, uint32_t buffer_size)
    : num_packets(num_packets), buffer_size(buffer_size)
    {
    }

    BufferSize()
    {
    }
  };

  struct DestBufferSize{
    std::map<uint8_t, uint32_t> harqIdBufferSize;
    std::map<uint32_t, BufferSize> packetIntervalBufferSize;

    DestBufferSize(std::map<uint8_t, uint32_t> harqIdBufferSize, std::map<uint32_t, BufferSize> packetIntervalBufferSize)
    : harqIdBufferSize(harqIdBufferSize), packetIntervalBufferSize(packetIntervalBufferSize)
    {
    }

    DestBufferSize()
    {
    }
  };

  // std::map<uint16_t, std::map<uint16_t, AggregatePacketDelayReportStruct>> AggregateData();
  // std::map<uint16_t, std::map<uint16_t, std::map<uint32_t, BufferSize>>> 
  std::map<uint16_t, std::map<uint16_t, NrGnbNetDevice::DestBufferSize>>
  AggregateData(std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itBegin,
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itEnd, 
                                        size_t numOfReports);
  // end modification

  const static uint16_t E2SM_REPORT_MAX_NEIGH = 8;

  

  struct SinrMcsPair
    {
        double sinr; ///< RNTI
        uint8_t mcs = 0; ///< DRBID

        SinrMcsPair(double sinr, uint8_t mcs)
        : sinr(sinr), mcs(mcs)
        {
        }

        SinrMcsPair()
        {
        }

        bool operator<(SinrMcsPair a) const{
          return a.sinr<sinr;
        }
    };

  struct E2SciData{
    uint64_t imsi;
    uint16_t rnti;
    NrSlSciF1aHeader sciF1Header;
    NrSlMacPduTag sciTag;
    double rsrpdBm;
    double x;
    double y;

    E2SciData(uint64_t imsi, uint16_t rnti, NrSlSciF1aHeader sciF1Header, NrSlMacPduTag sciTag, double rsrpdBm, double x, double y)
        : imsi(imsi), rnti(rnti), sciF1Header(sciF1Header),  sciTag(sciTag), rsrpdBm(rsrpdBm), x(x), y(y)
        {
        }
  };

  // generating cell, imsi, other cell and SinrMcsPair
  std::map<uint16_t, std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>> m_l3sinrMap;
  std::map<uint64_t, std::map<uint16_t, uint32_t>> m_ueUsedResourcesMap;

  
  std::map<uint16_t, std::vector<E2SciData> > m_e2SciDataMap;

  std::string GetImsiString(uint64_t imsi);

  uint32_t m_packetDelayGroupingGranularity;

  std::vector<uint32_t> m_acceptablePeriodicity = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};

  // source rnti, dest rnti, buffer delays
  

  void PrintPdu(E2AP_PDU *pdu_cucp_ue);

  std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>> m_e2PacketDelaysInBuffer;

  struct NodePosition{
    double x;
    double y;
    NodePosition()
    :x(0), y(0){}
    NodePosition(double x, double y)
    :x(x), y(y)
      {}
  };

  std::map<uint16_t, NodePosition> m_e2NodePositions;

  uint8_t* m_xappRemainingData;
  uint32_t m_xappRemainingDataSize{0};
  bool m_pauseSimulation{false};
  // end modification

};

}

#endif /* NR_ENB_NET_DEVICE_H */

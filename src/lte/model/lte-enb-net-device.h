/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
 *
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 * Author: Marco Miozzo <marco.miozzo@cttc.es> : Update to FF API Architecture
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it> : Integrated with new architecture - GSoC 2015 - Carrier Aggregation
 */

#ifndef LTE_ENB_NET_DEVICE_H
#define LTE_ENB_NET_DEVICE_H

#include "ns3/lte-net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/lte-phy.h"
#include "ns3/component-carrier-enb.h"
#include <ns3/oran-interface.h>
#include <vector>
#include <map>

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class LtePhy;
class LteEnbPhy;
class LteEnbMac;
class LteEnbRrc;
class FfMacScheduler;
class LteHandoverAlgorithm;
class LteAnr;
class LteFfrAlgorithm;
class LteEnbComponentCarrierManager;

/**
 * \ingroup lte
 *
 * The eNodeB device implementation
 */
class LteEnbNetDevice : public LteNetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  LteEnbNetDevice ();

  virtual ~LteEnbNetDevice (void);
  virtual void DoDispose (void);

  // inherited from NetDevice
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

  /**
   * \return a pointer to the MAC of the PCC.
   */
  Ptr<LteEnbMac> GetMac (void) const;

  /**
   * \param index CC index
   * \return a pointer to the MAC of the CC addressed by index.
   */
  Ptr<LteEnbMac> GetMac (uint8_t index) const;

  /**
   * \return a pointer to the physical layer of the PCC.
   */
  Ptr<LteEnbPhy> GetPhy (void) const;
  
  /**
   * \param index SCC index
   * \return a pointer to the physical layer of the SCC addressed by index.
   */
  Ptr<LteEnbPhy> GetPhy (uint8_t index) const;

  /** 
   * \return a pointer to the Radio Resource Control instance of the eNB
   */
  Ptr<LteEnbRrc> GetRrc () const;
  
  /** 
   * \return a pointer to the ComponentCarrierManager instance of the eNB
   */
  Ptr<LteEnbComponentCarrierManager> GetComponentCarrierManager () const;

  /** 
   * \return the Cell Identifier of this eNB
   */
  uint16_t GetCellId () const;

  /**
   * \return the identifiers of cells served by this eNB
   */
  std::vector<uint16_t> GetCellIds () const;

  /**
   * \param cellId cell ID
   * \return true if cellId is served by this eNB
   */
  bool HasCellId (uint16_t cellId) const;

  /** 
   * \return the uplink bandwidth in RBs
   */
  uint16_t GetUlBandwidth () const;

  /** 
   * \param bw the uplink bandwidth in RBs
   */
  void SetUlBandwidth (uint16_t bw);

  /** 
   * \return the downlink bandwidth in RBs
   */
  uint16_t GetDlBandwidth () const;

  /** 
   * \param bw the downlink bandwidth in RBs
   */
  void SetDlBandwidth (uint16_t bw);

  /** 
   * \return the downlink carrier frequency (EARFCN)
   */
  uint32_t GetDlEarfcn () const;

  /** 
   * \param earfcn the downlink carrier frequency (EARFCN)
   */
  void SetDlEarfcn (uint32_t earfcn);

  /** 
   * \return the uplink carrier frequency (EARFCN)
   */
  uint32_t GetUlEarfcn () const;

  /** 
   * \param earfcn the uplink carrier frequency (EARFCN)
   */
  void SetUlEarfcn (uint32_t earfcn);

  /**
   * \brief Returns the CSG ID of the eNodeB.
   * \return the Closed Subscriber Group identity
   * \sa LteEnbNetDevice::SetCsgId
   */
  uint32_t GetCsgId () const;

  /**
   * \brief Associate the eNodeB device with a particular CSG.
   * \param csgId the intended Closed Subscriber Group identity
   *
   * CSG identity is a number identifying a Closed Subscriber Group which the
   * cell belongs to. eNodeB is associated with a single CSG identity.
   *
   * The same CSG identity can also be associated to several UEs, which is
   * equivalent as enlisting these UEs as the members of this particular CSG.
   *
   * \sa LteEnbNetDevice::SetCsgIndication
   */
  void SetCsgId (uint32_t csgId);

  /**
   * \brief Returns the CSG indication flag of the eNodeB.
   * \return the CSG indication flag
   * \sa LteEnbNetDevice::SetCsgIndication
   */
  bool GetCsgIndication () const;

  /**
   * \brief Enable or disable the CSG indication flag.
   * \param csgIndication if TRUE, only CSG members are allowed to access this
   *                      cell
   *
   * When the CSG indication field is set to TRUE, only UEs which are members of
   * the CSG (i.e. same CSG ID) can gain access to the eNodeB, therefore
   * enforcing closed access mode. Otherwise, the eNodeB operates as a non-CSG
   * cell and implements open access mode.
   *
   * \note This restriction only applies to initial cell selection and
   *       EPC-enabled simulation.
   *
   * \sa LteEnbNetDevice::SetCsgIndication
   */
  void SetCsgIndication (bool csgIndication);

  /**
   * \brief Set the ComponentCarrier Map of the Enb
   * \param ccm the map of ComponentCarrierEnb
   *
   */

  void SetCcMap (std::map< uint8_t, Ptr<ComponentCarrierBaseStation> > ccm);

  /**
   * \returns  The Component Carrier Map of the Enb.
   *
   */

  std::map< uint8_t, Ptr<ComponentCarrierBaseStation> >  GetCcMap (void) const;

  // modified
  // public 
  void SetE2Termination (Ptr<E2Termination> e2term);

  Ptr<E2Termination> GetE2Termination() const;
  // end modification 

protected:
  // inherited from Object
  virtual void DoInitialize (void);


private:
  bool m_isConstructed; ///< is constructed?
  bool m_isConfigured; ///< is configured?

  /**
   * \brief Propagate attributes and configuration to sub-modules.
   *
   * Several attributes (e.g., the bandwidth) are exported as the attributes of
   * the LteEnbNetDevice from a user perspective, but are actually used also in
   * other sub-modules (the RRC, the PHY, the scheduler, etc.). This method
   * takes care of updating the configuration of all these sub-modules so that
   * their copy of attribute values are in sync with the one in
   * the LteEnbNetDevice.
   */
  void UpdateConfig ();

  Ptr<LteEnbRrc> m_rrc; ///< the RRC

  Ptr<LteHandoverAlgorithm> m_handoverAlgorithm; ///< the handover algorithm
 
  Ptr<LteAnr> m_anr; ///< ANR

  Ptr<LteFfrAlgorithm> m_ffrAlgorithm; /**< DEPRECATED - It is maintained for backward compatibility after adding CA feature*/

  uint16_t m_cellId; /**< Cell Identifier. Part of the CGI, see TS 29.274, section 8.21.1  */

  uint16_t m_dlBandwidth; /**<DEPRECATE - It is maintained for backward compatibility after adding CA feature- downlink bandwidth in RBs */
  uint16_t m_ulBandwidth; /**<DEPRECATE - It is maintained for backward compatibility after adding CA feature- uplink bandwidth in RBs */

  uint32_t m_dlEarfcn;  /**<DEPRECATE - It is maintained for backward compatibility after adding CA feature- downlink carrier frequency */
  uint32_t m_ulEarfcn;  /**<DEPRECATE - It is maintained for backward compatibility after adding CA feature- uplink carrier frequency */

  uint16_t m_csgId; ///< CSG ID
  bool m_csgIndication; ///< CSG indication

  std::map < uint8_t, Ptr<ComponentCarrierBaseStation> > m_ccMap; /**< ComponentCarrier map */
  
  Ptr<LteEnbComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager of this eNb

  // modified
  Ptr<E2Termination> m_e2term;
  std::string m_ltePlmnId;
  bool m_forceE2FileLogging; //< if true log PMs to files
  bool m_isReportingEnabled {false};
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

  void BuildAndSendReportMessage(E2Termination::RicSubscriptionRequest_rval_s params);

  Ptr<KpmIndicationHeader> BuildRicIndicationHeader(std::string plmId, std::string gnbId, uint16_t nrCellId);
  Ptr<KpmIndicationMessage> BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, size_t numOfReports);

  Ptr<KpmIndicationMessage> BuildRicIndicationMessageCuUp(std::string plmId);
  Ptr<KpmIndicationMessage> BuildRicIndicationMessageCuCp(std::string plmId);

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
  // generating cell, imsi, other cell and SinrMcsPair
  std::map<uint16_t, std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>> m_l3sinrMap;
  std::map<uint64_t, std::map<uint16_t, uint32_t>> m_ueUsedResourcesMap;

  std::string GetImsiString(uint64_t imsi);
  // end modification

}; // end of class LteEnbNetDevice

} // namespace ns3

#endif /* LTE_ENB_NET_DEVICE_H */

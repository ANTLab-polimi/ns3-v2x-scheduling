/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Northeastern University
 * Copyright (c) 2022 Sapienza, University of Rome
 * Copyright (c) 2022 University of Padova
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
 * Author: Andrea Lacava <thecave003@gmail.com>
 *		   Tommaso Zugno <tommasozugno@gmail.com>
 *		   Michele Polese <michele.polese@gmail.com>
 */

#ifndef ASN1C_TYPES_H
#define ASN1C_TYPES_H

#include "ns3/object.h"
#include <ns3/math.h>

extern "C" {
  #include "OCTET_STRING.h"
  #include "BIT_STRING.h"
  #include "PM-Info-Item.h"
  #include "SNSSAI.h"
  #include "RRCEvent.h"
  #include "L3-RRC-Measurements.h"
  #include "ServingCellMeasurements.h"
  #include "MeasResultNeighCells.h"
  #include "MeasResultNR.h"
  #include "MeasResultEUTRA.h"
  #include "MeasResultPCell.h"
  #include "MeasResultListEUTRA.h"
  #include "MeasResultListNR.h"
  #include "MeasResultServMO.h"
  #include "MeasResultServMOList.h"
  #include "MeasQuantityResults.h"
  #include "ResultsPerSSB-Index.h"
  #include "ResultsPerCSI-RS-Index.h"
  #include "E2SM-RC-ControlMessage-Format1.h"
  #include "RANParameter-Item.h"
  #include "RANParameter-ValueType.h"
  #include "RANParameter-ELEMENT.h"
  #include "RANParameter-STRUCTURE.h"
  #include "Buffer-String.h"

  #include "V2XId.h"
  #include "V2XAllUsersBufferDelayReports.h"
  #include "V2XAllBufferPacketDelays.h"
  #include "V2XBufferPacketDelays.h"
  #include "V2XPacketDelayInterval.h"
  #include "V2XSciMessageItem.h"
  #include "V2XSciMessageItem-List.h"
  #include "V2X-Single-User-report.h"
  #include "V2XHarqBufferSize.h"
}

namespace ns3 {

// modified
/**
* Wrapper for class for OCTET STRING  
*/
class BufferString : public SimpleRefCount<BufferString>
{
public:
  BufferString (std::string value, size_t size);
  BufferString (void *value, size_t size);
  ~BufferString ();
  Buffer_String_t *GetPointer ();
  Buffer_String_t GetValue ();
  std::string DecodeContent ();

private:
  void CreateBaseBufferString (size_t size);
  Buffer_String_t *m_bufferString;
};
// end modification

/**
* Wrapper for class for OCTET STRING  
*/
class OctetString : public SimpleRefCount<OctetString>
{
public:
  OctetString (std::string value, size_t size);
  OctetString (void *value, size_t size);
  ~OctetString ();
  OCTET_STRING_t *GetPointer ();
  OCTET_STRING_t GetValue ();
  std::string DecodeContent ();

private:
  void CreateBaseOctetString (size_t size);
  OCTET_STRING_t *m_octetString;
};

/**
* Wrapper for class for BIT STRING  
*/
class BitString : public SimpleRefCount<BitString>
{
public:
  BitString (std::string value, size_t size);
  BitString (std::string value, size_t size, size_t bits_unused);
  ~BitString ();
  BIT_STRING_t *GetPointer ();
  BIT_STRING_t GetValue ();
  // TODO maybe a to string or a decode method should be created

private:
  BIT_STRING_t *m_bitString;
};

class NrCellId : public SimpleRefCount<NrCellId>
{
public: 
  NrCellId (uint16_t value);
  virtual ~NrCellId ();
  BIT_STRING_t *GetPointer ();
  BIT_STRING_t GetValue ();
  
private: 
  Ptr<BitString> m_bitString;
};

/**
* Wrapper for class for S-NSSAI  
*/
class Snssai : public SimpleRefCount<Snssai>
{
public:
  Snssai (std::string sst);
  Snssai (std::string sst, std::string sd);
  ~Snssai ();
  SNSSAI_t *GetPointer ();
  SNSSAI_t GetValue ();

private:
  OCTET_STRING_t *m_sst;
  OCTET_STRING_t *m_sd;
  SNSSAI_t *m_sNssai;
};

/**
* Wrapper for class for MeasQuantityResults_t
*/
class MeasQuantityResultsWrap : public SimpleRefCount<MeasQuantityResultsWrap>
{
public:
  MeasQuantityResultsWrap ();
  ~MeasQuantityResultsWrap ();
  MeasQuantityResults_t *GetPointer ();
  MeasQuantityResults_t GetValue ();
  void AddRsrp (long rsrp);
  void AddRsrq (long rsrq);
  void AddSinr (long sinr);
  // modified
  void AddMcs(uint8_t mcs);
  // end modification

private:
  MeasQuantityResults_t *m_measQuantityResults;
};

/**
* Wrapper for class for ResultsPerCSI_RS_Index_t
*/
class ResultsPerCsiRsIndex : public SimpleRefCount<ResultsPerCsiRsIndex>
{
public:
  ResultsPerCsiRsIndex (long csiRsIndex, MeasQuantityResults_t *csiRsResults);
  ResultsPerCsiRsIndex (long csiRsIndex);
  ResultsPerCSI_RS_Index_t *GetPointer ();
  ResultsPerCSI_RS_Index_t GetValue ();

private:
  ResultsPerCSI_RS_Index_t *m_resultsPerCsiRsIndex;
};

/**
* Wrapper for class for ResultsPerSSB_Index_t
*/
class ResultsPerSSBIndex : public SimpleRefCount<ResultsPerSSBIndex>
{
public:
  ResultsPerSSBIndex (long ssbIndex, MeasQuantityResults_t *ssbResults);
  ResultsPerSSBIndex (long ssbIndex);
  ResultsPerSSB_Index_t *GetPointer ();
  ResultsPerSSB_Index_t GetValue ();

private:
  ResultsPerSSB_Index_t *m_resultsPerSSBIndex;
};

/**
* Wrapper for class for MeasResultNR_t
*/
class MeasResultNr : public SimpleRefCount<MeasResultNr>
{
public:
  enum ResultCell { SSB = 0, CSI_RS = 1 };
  MeasResultNr (long physCellId);
  MeasResultNr ();
  ~MeasResultNr ();
  MeasResultNR_t *GetPointer ();
  MeasResultNR_t GetValue ();
  void AddCellResults (ResultCell cell, MeasQuantityResults_t *results);
  void AddPerSsbIndexResults (ResultsPerSSB_Index_t *resultsSsbIndex);
  void AddPerCsiRsIndexResults (ResultsPerCSI_RS_Index_t *resultsCsiRsIndex);
  void AddPhyCellId (long physCellId);

private:
  MeasResultNR_t *m_measResultNr;
  bool m_shouldFree;
};

/**
* Wrapper for class for MeasResultEUTRA_t
*/
class MeasResultEutra : public SimpleRefCount<MeasResultEutra>
{
public:
  MeasResultEutra (long eutraPhysCellId, long rsrp, long rsrq, long sinr);
  MeasResultEutra (long eutraPhysCellId);
  MeasResultEUTRA_t *GetPointer ();
  MeasResultEUTRA_t GetValue ();
  void AddRsrp (long rsrp);
  void AddRsrq (long rsrq);
  void AddSinr (long sinr);

private:
  MeasResultEUTRA_t *m_measResultEutra;
};

/**
* Wrapper for class for MeasResultPCell_t
*/
class MeasResultPCellWrap : public SimpleRefCount<MeasResultPCellWrap>
{
public:
  MeasResultPCellWrap (long eutraPhysCellId, long rsrpResult, long rsrqResult);
  MeasResultPCellWrap (long eutraPhysCellId);
  MeasResultPCell_t *GetPointer ();
  MeasResultPCell_t GetValue ();
  void AddRsrpResult (long rsrpResult);
  void AddRsrqResult (long rsrqResult);

private:
  MeasResultPCell_t *m_measResultPCell;
};

/**
* Wrapper for class for MeasResultServMO_t
*/
class MeasResultServMo : public SimpleRefCount<MeasResultServMo>
{
public:
  MeasResultServMo (long servCellId, MeasResultNR_t measResultServingCell,
                    MeasResultNR_t *measResultBestNeighCell);
  MeasResultServMo (long servCellId, MeasResultNR_t measResultServingCell);
  MeasResultServMO_t *GetPointer ();
  MeasResultServMO_t GetValue ();

private:
  MeasResultServMO_t *m_measResultServMo;
};

/**
* Wrapper for class for ServingCellMeasurements_t
*/
class ServingCellMeasurementsWrap : public SimpleRefCount<ServingCellMeasurementsWrap>
{
public:
  ServingCellMeasurementsWrap (ServingCellMeasurements_PR present);
  ServingCellMeasurements_t *GetPointer ();
  ServingCellMeasurements_t GetValue ();
  void AddMeasResultPCell (MeasResultPCell_t *measResultPCell);
  void AddMeasResultServMo (MeasResultServMO_t *measResultServMO);

private:
  ServingCellMeasurements_t *m_servingCellMeasurements;
  MeasResultServMOList_t *m_nr_measResultServingMOList;
};

/**
* Wrapper for class for L3 RRC Measurements
*/
class L3RrcMeasurements : public SimpleRefCount<L3RrcMeasurements>
{
public:
  int MAX_MEAS_RESULTS_ITEMS = 8; // Maximum 8 per UE (standard)
  L3RrcMeasurements (RRCEvent_t rrcEvent);
  L3RrcMeasurements (L3_RRC_Measurements_t *l3RrcMeasurements);
  ~L3RrcMeasurements ();
  L3_RRC_Measurements_t *GetPointer ();
  L3_RRC_Measurements_t GetValue ();

  void AddMeasResultEUTRANeighCells (MeasResultEUTRA_t *measResultItemEUTRA);
  void AddMeasResultNRNeighCells (MeasResultNR_t *measResultItemNR);
  void AddServingCellMeasurement (ServingCellMeasurements_t *servingCellMeasurements);
  void AddNeighbourCellMeasurement (long neighCellId, long sinr);


  static Ptr<L3RrcMeasurements> CreateL3RrcUeSpecificSinrServing (long servingCellId,
                                                                  long physCellId, long sinr);

  // modified
  void AddNeighbourCellMeasurementMcs (long neighCellId, long sinr, uint8_t mcs);
  static Ptr<L3RrcMeasurements> CreateL3RrcUeSpecificSinrMcsServing (long servingCellId,
                                                                  long physCellId, long sinr, uint8_t mcs);
  // end modification

  static Ptr<L3RrcMeasurements> CreateL3RrcUeSpecificSinrNeigh ();

  // TODO change definition and return the values (to be used for decoding)
  static void ExtractMeasurementsFromL3RrcMeas (L3_RRC_Measurements_t *l3RrcMeasurements);
  
  /**
   * Returns the input SINR on a 0-127 scale
   * 
   * Refer to 3GPP TS 38.133 V17.2.0(2021-06), Table 10.1.16.1-1: SS-SINR and CSI-SINR measurement report mapping
   * 
   * @param sinr 
   * @return double 
   */
  static double ThreeGppMapSinr (double sinr);

private:
  void addMeasResultNeighCells (MeasResultNeighCells_PR present);
  L3_RRC_Measurements_t *m_l3RrcMeasurements;
  MeasResultListEUTRA_t *m_measResultListEUTRA;
  MeasResultListNR_t *m_measResultListNR;
  int m_measItemsCounter;
};
/**
 * @brief wrapper for the sci messages
 */



class V2XUserReceivedSciMessages : public SimpleRefCount<V2XUserReceivedSciMessages>
{
  public:
  V2XUserReceivedSciMessages ();
  void addReceivedSciMessage(uint8_t* sciHeaderBuffer, uint32_t sciHeaderBufferLength, 
                              uint8_t* sciTagBuffer, uint32_t sciTagBufferLength, double rsrp);
  void addReceivedSciMessage(V2XSciMessageItem_t* sciMessageItem);
  V2XSciMessageItemList_t *GetPointer ();
  V2XSciMessageItemList_t GetValue ();

private:
  V2XSciMessageItemList_t *m_v2XUserReceivedSciMessages;
};

/**
 * @brief wrapper for the insertion of a single packe delay interval in the list
 * 
 */


class V2XPacketDelayIntervalWrap : public SimpleRefCount<V2XPacketDelayIntervalWrap>
{
public:
  V2XPacketDelayIntervalWrap (long lowerInterval, long upperInterval, long numberOfPackets, long reservationPeriodMs, long bufferSize);
  V2XPacketDelayInterval_t *GetPointer ();
  V2XPacketDelayInterval_t GetValue ();

private:
  V2XPacketDelayInterval_t *m_v2XPacketDelayInterval;
};

class V2XHarqBufferSizeWrap : public SimpleRefCount<V2XHarqBufferSizeWrap>
{
public:
  V2XHarqBufferSizeWrap (long harqId, long numberOfPackets, long bufferSize);
  V2XHarqBufferSize_t *GetPointer ();
  V2XHarqBufferSize_t GetValue ();

private:
  V2XHarqBufferSize_t *m_v2XHarqBufferSize;
};

class V2XBufferPacketDelayIntervalsWrap : public SimpleRefCount<V2XBufferPacketDelayIntervalsWrap>
{
public:
  V2XBufferPacketDelayIntervalsWrap (long rnti); 
  void AddPacketInterval (V2XPacketDelayInterval_t *v2XPacketDelayInterval);
  void AddHarqBufferSize (V2XHarqBufferSize_t *v2xHarqBufferSize);
  V2XBufferPacketDelays_t *GetPointer ();
  V2XBufferPacketDelays_t GetValue ();

private:
  V2XBufferPacketDelays_t *m_v2XBufferPacketDelay;
};

/**
* Wrapper for class for V2X Buffer delays
*/
class V2XUserBufferPacketDelays : public SimpleRefCount<V2XUserBufferPacketDelays>
{
public:
  V2XUserBufferPacketDelays (long rnti);
  void AddBufferPacketDelays (V2XBufferPacketDelays_t *v2XBufferPacketDelays);
  // ~V2XUserBufferPacketDelays ();
  V2XAllBufferPacketDelays_t *GetPointer ();
  V2XAllBufferPacketDelays_t GetValue ();


private:
  // the all buffer class
  V2XAllBufferPacketDelays_t *m_v2xAllBufferPacketDelays;
};

/**
 * @brief 
 * 
 */
class V2XAllUsersBufferPacketDelays : public SimpleRefCount<V2XAllUsersBufferPacketDelays>
{
public:
  V2XAllUsersBufferPacketDelays ();
  void AddUserBufferPacketDelays (V2XAllBufferPacketDelays_t *v2XUserBufferPacketDelays);
  V2XAllUsersBufferDelayReports_t *GetPointer ();
  V2XAllUsersBufferDelayReports_t GetValue ();


private:
  // the all buffer class
  V2XAllUsersBufferDelayReports_t *m_v2xAllUsersBufferPacketDelays;
};


class V2XSingleReportWrap: public SimpleRefCount<V2XSingleReportWrap>{
  public:
  V2XSingleReportWrap (double x, double y);
  V2XSingleReportWrap (V2XAllBufferPacketDelays_t *v2XUserBufferPacketDelays, V2XSciMessageItemList_t *v2XUserReceivedSciMessages, double x, double y);
  void AddUserBufferPacketDelays (V2XAllBufferPacketDelays_t *v2XUserBufferPacketDelays);
  void AddUserReceivedSciMessageList (V2XSciMessageItemList_t *v2XUserReceivedSciMessages);
  void AddUserBufferAndSciMessageList (V2XAllBufferPacketDelays_t *v2XUserBufferPacketDelays, V2XSciMessageItemList_t *v2XUserReceivedSciMessages);
  
  V2X_Single_User_Report_t *GetPointer ();
  V2X_Single_User_Report_t GetValue ();


private:
  // the all buffer class
  V2X_Single_User_Report_t *m_v2xSingleUserReport;
  
};

/**
* Wrapper for class for PM_Info_Item_t
*/
class MeasurementItem : public SimpleRefCount<MeasurementItem>
{
public:
  MeasurementItem (std::string name, long value);
  MeasurementItem (std::string name, double value);
  MeasurementItem (std::string name, Ptr<L3RrcMeasurements> value);
  MeasurementItem (std::string name, Ptr<OctetString> value);
  MeasurementItem (std::string name, Ptr<BufferString> value);
  // MeasurementItem (std::string name, Ptr<V2XAllUsersBufferPacketDelays> value);
  // MeasurementItem (std::string name, Ptr<V2XUserReceivedSciMessages> value);
  MeasurementItem (std::string name, Ptr<V2XSingleReportWrap> value);
  ~MeasurementItem ();
  PM_Info_Item_t *GetPointer ();
  PM_Info_Item_t GetValue ();

private:
  MeasurementItem (std::string name);
  void CreateMeasurementValue (MeasurementValue_PR measurementValue_PR);
  // Main struct to be compiled
  PM_Info_Item_t *m_measurementItem;

  // Accessory structs that we must track to release memory after use
  MeasurementTypeName_t *m_measName;
  MeasurementValue_t *m_pmVal;
  MeasurementType_t *m_pmType;
};

/**
* Wrapper for class for RANParameter_Item_t 
*/
class RANParameterItem : public SimpleRefCount<RANParameterItem>
{
public:
  enum ValueType{ Nothing = 0, Int = 1, OctectString = 2 };
  RANParameterItem (RANParameter_Item_t *ranParameterItem);
  ~RANParameterItem ();
  RANParameter_Item_t *GetPointer ();
  RANParameter_Item_t GetValue ();

  ValueType m_valueType;
  long m_valueInt;
  Ptr<OctetString> m_valueStr;

  static std::vector<RANParameterItem>
  ExtractRANParametersFromRANParameter (RANParameter_Item_t *ranParameterItem);

private:
  // Main struct
  RANParameter_Item_t *m_ranParameterItem;
  BOOLEAN_t *m_keyFlag;
};

} // namespace ns3
#endif /* ASN1C_TYPES_H */

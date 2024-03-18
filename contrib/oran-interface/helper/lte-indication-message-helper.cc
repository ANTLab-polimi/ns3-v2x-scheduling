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


#include <ns3/lte-indication-message-helper.h>

namespace ns3 {

LteIndicationMessageHelper::LteIndicationMessageHelper (IndicationMessageType type, bool isOffline,
                                                        bool reducedPmValues)
    : IndicationMessageHelper (type, isOffline, reducedPmValues)
{
  NS_ABORT_MSG_IF (type == IndicationMessageType::Du,
                   "Wrong type for LTE Indication Message, expected CuUp or CuCp");
}

void
LteIndicationMessageHelper::AddCuUpUePmItem (std::string ueImsiComplete, long txBytes,
                                             long txDlPackets, double pdcpThroughput,
                                             double pdcpLatency)
{
  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);

  if (!m_reducedPmValues)
    {
      // UE-specific PDCP SDU volume from LTE eNB. Unit is Mbits
      ueVal->AddItem<long> ("DRB.PdcpSduVolumeDl_Filter.UEID", txBytes);

      // UE-specific number of PDCP SDUs from LTE eNB
      ueVal->AddItem<long> ("Tot.PdcpSduNbrDl.UEID", txDlPackets);

      // UE-specific Downlink IP combined EN-DC throughput from LTE eNB. Unit is kbps
      ueVal->AddItem<double> ("DRB.PdcpSduBitRateDl.UEID", pdcpThroughput);

      //UE-specific Downlink IP combined EN-DC throughput from LTE eNB
      ueVal->AddItem<double> ("DRB.PdcpSduDelayDl.UEID", pdcpLatency);
    }

  m_msgValues.m_ueIndications.insert (ueVal);
}

void
LteIndicationMessageHelper::AddCuUpCellPmItem (double cellAverageLatency)
{
  if (!m_reducedPmValues)
    {
      Ptr<MeasurementItemList> cellVal = Create<MeasurementItemList> ();
      cellVal->AddItem<double> ("DRB.PdcpSduDelayDl", cellAverageLatency);
      m_msgValues.m_cellMeasurementItems = cellVal;
    }
}

void
LteIndicationMessageHelper::FillCuUpValues (std::string plmId, long pdcpBytesUl, long pdcpBytesDl)
{
  m_cuUpValues->m_pDCPBytesUL = pdcpBytesUl;
  m_cuUpValues->m_pDCPBytesDL = pdcpBytesDl;
  FillBaseCuUpValues (plmId);
}

void
LteIndicationMessageHelper::FillCuCpValues (uint16_t numActiveUes)
{
  FillBaseCuCpValues (numActiveUes);
}

void
LteIndicationMessageHelper::AddCuCpUePmItem (std::string ueImsiComplete, long numDrb,
                                             long drbRelAct)
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);
  if (!m_reducedPmValues)
    {
      ueVal->AddItem<long> ("DRB.EstabSucc.5QI.UEID", numDrb);
      ueVal->AddItem<long> ("DRB.RelActNbr.5QI.UEID", drbRelAct); // not modeled in the simulator
    }
  m_msgValues.m_ueIndications.insert (ueVal);
}

// modified
void
LteIndicationMessageHelper::AddCuCpUePmItem (std::string ueImsiComplete, long assignedCellId,
                                                long usedResources,
                                                Ptr<L3RrcMeasurements> l3RrcMeasurementServing,
                                                Ptr<L3RrcMeasurements> l3RrcMeasurementNeigh)
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);

  ueVal->AddItem<long> ("AssignedCell.CellId.UEID", assignedCellId);
  ueVal->AddItem<long> ("AssignedCell.UsedResources.UEID", usedResources); // not modeled in the simulator

  ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.SrcCellQual.RS-SINR.UEID", l3RrcMeasurementServing);
  ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.TrgtCellQual.RS-SINR.UEID", l3RrcMeasurementNeigh);

  m_msgValues.m_ueIndications.insert (ueVal);
}

// adding the struct as serailized data
void
LteIndicationMessageHelper::AddCuCpUePmItem (std::string ueImsiComplete, long assignedCellId, 
                                              uint8_t* sciHeaderBuffer, uint32_t sciHeaderBufferLength, 
                                              uint8_t* sciTagBuffer, uint32_t sciTagBufferLength,
                                              double rsrpdBm, double x, double y)
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);
  // std::cout << " Creating octet string with the buffer " << std::endl;
  Ptr<BufferString> sciHeader = Create<BufferString> ((void *)sciHeaderBuffer, sciHeaderBufferLength);
  Ptr<BufferString> sciTag = Create<BufferString> ((void *) sciTagBuffer, sciTagBufferLength);

  ueVal->AddItem<Ptr<BufferString>> ("SciHeaderBuffer", sciHeader);
  ueVal->AddItem<Ptr<BufferString>> ("SciTagBuffer", sciTag);
  
  ueVal->AddItem<double> ("RsrpdBm", rsrpdBm);
  ueVal->AddItem<double> ("PosX", x);
  ueVal->AddItem<double> ("PosY", y);

  // inserting the map of delays for the user

  m_msgValues.m_ueIndications.insert (ueVal);
}

void
LteIndicationMessageHelper::AddCuCpUePmItem (std::string ueImsiComplete, long assignedCellId, 
                                              Ptr<V2XSingleReportWrap> singleReportPtr,
                                              double x, double y, uint16_t frame, uint8_t subframe, uint16_t slot,
                                              uint64_t timestamp)
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);

  ueVal->AddItem<double> ("PosX", x);
  ueVal->AddItem<double> ("PosY", y);

  ueVal->AddItem<long> ("Frame", frame);
  ueVal->AddItem<long> ("SubFrame", subframe);
  ueVal->AddItem<long> ("Slot", slot);

  ueVal->AddItem<long> ("Timestamp", (long)timestamp);

  ueVal->AddItem<Ptr<V2XSingleReportWrap>> ("SingleReport", singleReportPtr);

  // ueVal->AddItem<long> ("UEID.Rnti", y);
  // ueVal->AddItem<long> ("UEID.Imsi", y);

  // xer_fprint(stdout, &asn_DEF_V2XSciMessageItemList,  (void*) v2xUserReceivedSciMessages->GetPointer());
  

  // inserting the map of delays for the user

  m_msgValues.m_ueIndications.insert (ueVal);
}

void
LteIndicationMessageHelper::AddCuCpUePmItem (std::string ueImsiComplete, long assignedCellId,
                                                long usedResources,
                                                Ptr<L3RrcMeasurements> l3RrcMeasurementServing,
                                                Ptr<L3RrcMeasurements> l3RrcMeasurementNeigh,
                                                bool isElastic, uint16_t numerology)
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);
  long isElasticLong = 0;
  if(isElastic){
    isElasticLong = 1;
  }
  // for (int i = 0; i<10; i++){

    ueVal->AddItem<long> ("AssignedCell.CellId.UEID", assignedCellId);
    ueVal->AddItem<long> ("AssignedCell.UsedResources.UEID", usedResources); // not modeled in the simulator
    ueVal->AddItem<long> ("AssignedCell.ElasticUser.UEID", isElasticLong); // not modeled in the simulator
    ueVal->AddItem<long> ("AssignedCell.Numerology.UEID", numerology); // not modeled in the simulator

    ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.SrcCellQual.RS-SINR.UEID", l3RrcMeasurementServing);
    ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.TrgtCellQual.RS-SINR.UEID", l3RrcMeasurementNeigh);

    // for (int i =0; i< 10; i++){
    //   ueVal->AddItem<long> ("AssignedCell.UsedResources.UEID" + std::to_string(i), i);
    // }

    // ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO1.TrgtCellQual.RS-SINR.UEID1", l3RrcMeasurementNeigh);
    // ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO3.TrgtCellQual.RS-SINR.UEID2", l3RrcMeasurementNeigh);
    // ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO2.TrgtCellQual.RS-SINR.UEID3", l3RrcMeasurementNeigh);
    // ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO4.TrgtCellQual.RS-SINR.UEID", l3RrcMeasurementNeigh);
    // ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO5.TrgtCellQual.RS-SINR.UEID", l3RrcMeasurementNeigh);
  // }

  m_msgValues.m_ueIndications.insert (ueVal);
}

// detection phase
void
LteIndicationMessageHelper::AddDetectionCuUpUePmItem (std::string ueImsiComplete, long frameNum,
                                              long dataSym, long headoflinedelaynewtx, 
                                              double currenttxqueuesize, long tbSize,
                                              long scheduledUes, long frameUsedDataSym)
{
  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);

  if (!m_reducedPmValues)
  {
    // Frame number the report is generated
    ueVal->AddItem<long> ("FNum", frameNum);

    // Head of line delay new tx
    ueVal->AddItem<long> ("HOLTX", headoflinedelaynewtx);

    // Current tx queue
    ueVal->AddItem<double> ("TxQ", currenttxqueuesize);

    // Transport block size
    ueVal->AddItem<long> ("TbS", tbSize);

    // Data symbols
    ueVal->AddItem<long> ("DSym", dataSym);

    // Scheduled ues
    ueVal->AddItem<long> ("SchdUes", scheduledUes);

    // Frame used symbols
    ueVal->AddItem<long> ("FDSym", frameUsedDataSym);
  }

  m_msgValues.m_ueIndications.insert (ueVal);
}

// end modification

LteIndicationMessageHelper::~LteIndicationMessageHelper ()
{
}

} // namespace ns3
/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#include "nr-sl-phy-mac-common.h"

namespace ns3 {

bool
NrSlSlotAlloc::operator < (const NrSlSlotAlloc &rhs) const
{
  return (sfn < rhs.sfn);
}

bool
NrSlVarTtiAllocInfo::operator < (const NrSlVarTtiAllocInfo &rhs) const
{
  return (symStart < rhs.symStart);
}

// modified
void 
NrSlSlotAlloc::SerializeForE2 (Buffer::Iterator i) const
{
  // Buffer::Iterator i = start;
  // NS_LOG_FUNCTION(this);
  // NS_LOG_DEBUG("Writing");
  // the vector
  // first encode the number of elements in the vector
  i.WriteU32 ((uint32_t) slRlcPduInfo.size());
  for (auto &vecElem: slRlcPduInfo){
    i.WriteU8 (vecElem.lcid);
    i.WriteU32 (vecElem.size);
  }

  i.WriteU64 (SfnSf::Encode(sfn));
  // NS_LOG_DEBUG("Writing");
  i.WriteU32 (dstL2Id);
  i.WriteU8 (ndi);
  i.WriteU8 (rv);
  i.WriteU8 (priority);
  i.WriteU16 (mcs);
  //PSCCH
  i.WriteU16 (numSlPscchRbs);
  i.WriteU16 (slPscchSymStart);
  i.WriteU16 (slPscchSymLength);
  //PSSCH
  i.WriteU16 (slPsschSymStart);
  i.WriteU16 (slPsschSymLength);
  i.WriteU16 (slPsschSubChStart);
  i.WriteU16 (slPsschSubChLength);

  i.WriteU16 (maxNumPerReserve);
  i.WriteU8 ((uint8_t) txSci1A);
  i.WriteU8 (slotNumInd);

}

uint32_t 
NrSlSlotAlloc::DeserializeForE2 (Buffer::Iterator i)
{
  // Buffer::Iterator i = start;

  slRlcPduInfo.clear();
  uint32_t vecSize =  i.ReadU32 ();
  for (uint32_t _ind = 0; _ind< vecSize; _ind++){
    uint8_t lcid = i.ReadU8 ();
    uint32_t size = i.ReadU32 ();
    slRlcPduInfo.push_back(SlRlcPduInfo(lcid, size));
  }

  sfn = SfnSf::Decode(i.ReadU64 ());
  // NS_LOG_DEBUG("Writing");
  dstL2Id = i.ReadU32 ();
  ndi = i.ReadU8 ();
  rv = i.ReadU8 ();
  priority = i.ReadU8 ();
  mcs = i.ReadU16 ();
  //PSCCH
  numSlPscchRbs = i.ReadU16 ();
  slPscchSymStart = i.ReadU16 ();
  slPscchSymLength = i.ReadU16 ();
  //PSSCH
  slPsschSymStart = i.ReadU16 ();
  slPsschSymLength = i.ReadU16 ();
  slPsschSubChStart = i.ReadU16 ();
  slPsschSubChLength = i.ReadU16 ();

  maxNumPerReserve = i.ReadU16 ();
  txSci1A = (bool)i.ReadU8 ();
  slotNumInd = i.ReadU8 ();
    

  return GetSerializedSizeForE2 ();
}

uint32_t
NrSlSlotAlloc::GetSerializedSizeForE2 (void) const
{
  uint32_t totalSize = 4 + 5*slRlcPduInfo.size() + 
                    8 + 4 + 1 + 1 +1 + 2 +
                    2 + 2 + 2 +
                    2 + 2 + 2 + 2 +
                    2 + 1 + 1;
  return totalSize;
}
// end modification
}

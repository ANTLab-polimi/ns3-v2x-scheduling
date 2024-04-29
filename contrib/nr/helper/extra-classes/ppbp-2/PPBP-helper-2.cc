/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Doreid AMMAR - INRIA RESO team, LIP laboratory
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
 * Author: Doreid Ammar <doreid.ammar@gmail.com>
 */

#include "PPBP-helper-2.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/string.h"
#include "ns3/names.h"

namespace ns3 {

PPBPHelper2::PPBPHelper2 (std::string protocol, Address address, Address address2)
{
  m_factory.SetTypeId ("ns3::PPBPApplication2");
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("Remote", AddressValue (address));
  m_factory.Set ("Remote2", AddressValue (address2));
}

void 
PPBPHelper2::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
PPBPHelper2::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
PPBPHelper2::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
PPBPHelper2::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
PPBPHelper2::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3

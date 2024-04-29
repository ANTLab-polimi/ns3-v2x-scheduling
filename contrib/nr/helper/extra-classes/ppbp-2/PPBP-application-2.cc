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
 * Author: Sharan Naribole <nsharan@rice.edu>
 * Extended from PPBP Application provided for older ns-3 version at
 * http://perso.ens-lyon.fr/thomas.begin/NS3-PPBP.zip
 */

#include "PPBP-application-2.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE ("PPBPApplication2");

using namespace std;

namespace ns3 {

	NS_OBJECT_ENSURE_REGISTERED (PPBPApplication2);

	TypeId
	PPBPApplication2::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::PPBPApplication2")
		.SetParent<Application> ()
		.AddConstructor<PPBPApplication2> ()
		.AddAttribute ("BurstIntensity", "The data rate of each burst.",
					   DataRateValue (DataRate ("1Mb/s")),
					   MakeDataRateAccessor (&PPBPApplication2::m_cbrRate),
					   MakeDataRateChecker ())
		.AddAttribute ("PacketSize", "The size of packets sent in on state",
					   UintegerValue (1470),
					   MakeUintegerAccessor (&PPBPApplication2::m_pktSize),
					   MakeUintegerChecker<uint32_t> (1))
		.AddAttribute ("MeanBurstArrivals", "Mean Active Sources",
					   StringValue ("ns3::ConstantRandomVariable[Constant=20.0]"),
                   	   MakePointerAccessor (&PPBPApplication2::m_burstArrivals),
                       MakePointerChecker <RandomVariableStream>())
		.AddAttribute ("MeanBurstTimeLength", "Pareto distributed burst durations",
					   StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"),
                   	   MakePointerAccessor (&PPBPApplication2::m_burstLength),
                       MakePointerChecker <RandomVariableStream>())
		.AddAttribute ("H", "Hurst parameter",
					   DoubleValue (0.7),
					   MakeDoubleAccessor (&PPBPApplication2::m_h),
					   MakeDoubleChecker<double> ())
		.AddAttribute ("Remote", "The address of the destination",
					   AddressValue (),
					   MakeAddressAccessor (&PPBPApplication2::m_peer),
					   MakeAddressChecker ())
		.AddAttribute ("Remote2", "The address of the destination",
					   AddressValue (),
					   MakeAddressAccessor (&PPBPApplication2::m_peer2),
					   MakeAddressChecker ())
		.AddAttribute ("Protocol", "The type of protocol to use.",
					   TypeIdValue (UdpSocketFactory::GetTypeId ()),
					   MakeTypeIdAccessor (&PPBPApplication2::m_protocolTid),
					   MakeTypeIdChecker ())
		.AddTraceSource ("Tx", "A new packet is created and is sent",
						 MakeTraceSourceAccessor (&PPBPApplication2::m_txTrace),
						 "ns3::Packet::TracedCallback")
		;
		return tid;
	}

	PPBPApplication2::PPBPApplication2 ()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_socket = 0;
		m_connected = false;
		// modified
		m_socket2 = 0;
		m_connected2 = false;
		// end modification
		m_lastStartTime = Seconds (0);
		m_totalBytes1 = 0;
		m_totalBytes2 = 0;
		m_activebursts = 0;
		m_offPeriod = true;
	}

	PPBPApplication2::~PPBPApplication2()
	{
		NS_LOG_FUNCTION_NOARGS ();
	}

	uint32_t
	PPBPApplication2::GetTotalBytes() const
	{
		return m_totalBytes1;
	}

	void
	PPBPApplication2::DoDispose (void)
	{
		NS_LOG_FUNCTION_NOARGS ();

		m_socket = 0;
		// modified
		m_socket2 = 0;
		// end modification
		// chain up
		Application::DoDispose ();
	}

	// Application Methods
	void
	PPBPApplication2::StartApplication() // Called at time specified by Start
	{
		NS_LOG_FUNCTION_NOARGS ();

		// Create the socket if not already
		if (!m_socket)
		{
			m_socket = Socket::CreateSocket (GetNode(), m_protocolTid);
			m_socket->Bind ();
			m_socket->Connect (m_peer);
		}
		if (!m_socket2)
		{
			m_socket2 = Socket::CreateSocket (GetNode(), m_protocolTid);
			m_socket2->Bind ();
			m_socket2->Connect (m_peer2);
		}
		// Insure no pending event
		CancelEvents ();
		ScheduleStartEvent();
	}

	void
	PPBPApplication2::PPBP() // Poisson Pareto Burst
	{
		NS_LOG_FUNCTION_NOARGS ();

		double inter_burst_intervals;
		inter_burst_intervals = (double) 1/m_burstArrivals->GetValue ();

		Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
    	exp->SetAttribute ("Mean", DoubleValue (inter_burst_intervals));

		Time t_poisson_arrival = Seconds (exp->GetValue());
		m_PoissonArrival = Simulator::Schedule(t_poisson_arrival,&PPBPApplication2::PoissonArrival, this);

		// Pareto
		m_shape = 3 - 2 * m_h;
		m_timeSlot = Seconds((double) (m_shape - 1) * m_burstLength->GetValue () / m_shape);

		Ptr<ParetoRandomVariable> pareto = CreateObject<ParetoRandomVariable> ();
		pareto->SetAttribute ("Scale", DoubleValue (m_burstLength->GetValue ()));
		pareto->SetAttribute ("Shape", DoubleValue (m_shape));

    	double t_pareto = pareto->GetValue ();

		m_ParetoDeparture = Simulator::Schedule(t_poisson_arrival + Seconds (t_pareto),&PPBPApplication2::ParetoDeparture, this);
		m_ppbp = Simulator::Schedule(t_poisson_arrival,&PPBPApplication2::PPBP, this);
	}

	void PPBPApplication2::PoissonArrival()
	{
		NS_LOG_FUNCTION_NOARGS ();
		++m_activebursts;
		if (m_offPeriod) ScheduleNextTx();
	}

	void
	PPBPApplication2::ParetoDeparture()
	{
		NS_LOG_FUNCTION_NOARGS ();
		--m_activebursts;
	}

	void
	PPBPApplication2::StopApplication() // Called at time specified by Stop
	{
		NS_LOG_FUNCTION_NOARGS ();

		CancelEvents ();
		if(m_socket != 0) m_socket->Close ();
		// modified
		if(m_socket2 != 0) m_socket2->Close ();
		// end modification
		else NS_LOG_WARN("PPBPApplication2 found null socket to close in StopApplication");
	}

	void
	PPBPApplication2::CancelEvents ()
	{
		NS_LOG_FUNCTION_NOARGS ();
		Simulator::Cancel(m_sendEvent);
		Simulator::Cancel(m_startStopEvent);

		Simulator::Cancel(m_ppbp);
		Simulator::Cancel(m_PoissonArrival);
		Simulator::Cancel(m_ParetoDeparture);
	}

	// Event handlers
	void
	PPBPApplication2::StartSending()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_lastStartTime = Simulator::Now();
		ScheduleNextTx();					// Schedule the send packet event
		ScheduleStopEvent();
	}

	void
	PPBPApplication2::StopSending()
	{
		NS_LOG_FUNCTION_NOARGS ();
		CancelEvents();

		ScheduleStartEvent();
	}

	void
	PPBPApplication2::ScheduleNextTx()
	{
		NS_LOG_FUNCTION_NOARGS ();
		uint32_t bits = (m_pktSize + 30) * 8;
		Time nextTime(Seconds (bits /
							   static_cast<double>(m_cbrRate.GetBitRate())));

		if (m_activebursts != 0)
		{
			m_offPeriod = false;
			double data_rate = (double) nextTime.GetSeconds() / m_activebursts;
			m_sendEvent = Simulator::Schedule(Seconds(data_rate),&PPBPApplication2::SendPacket, this);
		}
		else
		{
			m_offPeriod = true;
		}

	}

	void
	PPBPApplication2::ScheduleStartEvent()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_ppbp = Simulator::Schedule(Seconds(0.0), &PPBPApplication2::PPBP, this);
		m_startStopEvent = Simulator::Schedule(Seconds(0.0), &PPBPApplication2::StartSending, this);
	}

	void
	PPBPApplication2::ScheduleStopEvent()
	{
		NS_LOG_FUNCTION_NOARGS ();
	}

	void
	PPBPApplication2::SendPacket()
	{
		NS_LOG_FUNCTION_NOARGS ();
		if(m_totalBytes1<m_totalBytes2){
			Ptr<Packet> packet = Create<Packet> (m_totalBytes2-m_totalBytes1);
			m_txTrace (packet);
			int ret1 = m_socket->Send (packet);
			if(ret1>0){
				m_totalBytes1 += packet->GetSize();
			}
		}else if(m_totalBytes1>m_totalBytes2){
			Ptr<Packet> packet = Create<Packet> (m_totalBytes1-m_totalBytes2);
			m_txTrace (packet);
			int ret2 = m_socket2->Send (packet);
			if(ret2>0){
				m_totalBytes2 += packet->GetSize();
			}
		}else{
			Ptr<Packet> packet = Create<Packet> (m_pktSize);
			m_txTrace (packet);
			int ret1 = m_socket->Send (packet);
			// modified
			int ret2 = m_socket2->Send (packet);
			// end modification
			if(ret1>0){
				m_totalBytes1 += packet->GetSize();
			}
			if(ret2>0){
				m_totalBytes2 += packet->GetSize();
			}
			m_lastStartTime = Simulator::Now();
		}

		// Ptr<Packet> packet = Create<Packet> (m_pktSize);
		// m_txTrace (packet);
		// int ret1 = m_socket->Send (packet);
		// // modified
		// int ret2 = m_socket2->Send (packet);
		// // end modification
		// if(ret1>0){
		// 	m_totalBytes1 += packet->GetSize();
		// }
		// if(ret2>0){
		// 	m_totalBytes2 += packet->GetSize();
		// }

		m_lastStartTime = Simulator::Now();
		
		ScheduleNextTx();
	}

	void
	PPBPApplication2::ConnectionSucceeded(Ptr<Socket>)
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_connected = true;
		// modified 
		m_connected2 = true;
		// end modification
		ScheduleStartEvent();
	}

	void
	PPBPApplication2::ConnectionFailed(Ptr<Socket>)
	{
		NS_LOG_FUNCTION_NOARGS ();
		cout << "PPBPApplication2, Connection Failed" << endl;
	}
} // Namespace ns3


#include "util.h"
#include "distComm.h"
#include "distCntxt.h"
#include "distRequest.h"
#include "object.h"
#include "debug.h"
#include "events.h"
#include "eventChannel.h"

using namespace PowerAPI;

uint64_t DistComm::m_currentCommID = 1;
DistComm::DistComm( DistCntxt* cntxt, std::set<Object*>& objects )
{
	DBGX("num objects %lu\n", objects.size() );
	m_ec = cntxt->getEventChannel();

	DBGX("\n");
	CommCreateEvent* ev = new CommCreateEvent();
	DBGX("\n");
	m_commID = ev->commID = m_currentCommID++;

	std::set<Object*>::iterator iter = objects.begin();
	for ( ; iter != objects.end(); ++iter ) {
		ev->members.push_back((*iter)->name());
	}

	DBGX("\n");
	m_ec->sendEvent( ev );
	DBGX("\n");
}

void DistComm::getValue( PWR_AttrName attr, CommReq* req )
{
	DBGX("%s\n",attrNameToString(attr));

	DistGetCommReq* distReq = static_cast<DistGetCommReq*>(req);

	CommReqEvent* ev = new CommReqEvent;	
	ev->commID = m_commID;
	ev->op = CommEvent::Get;
	ev->id = (EventId) req;	
	ev->attrName = attr; 
	m_ec->sendEvent( ev );
	delete ev;
}

void DistGetCommReq::process( Event* _ev ) {
	m_req->getValue( this, static_cast<CommRespEvent*>(_ev) );
}

void DistComm::setValue( PWR_AttrName attr, void* buf, CommReq* req )
{
	DBGX("%s\n",attrNameToString(attr));
	DistSetCommReq* distReq = static_cast<DistSetCommReq*>(req);

	CommReqEvent* ev = new CommReqEvent;	
	ev->commID = m_commID;
	ev->op = CommEvent::Set;
	ev->id = (EventId) req;	
	ev->attrName = attr; 
	ev->value = *(uint64_t*)buf;
	m_ec->sendEvent( ev );
	delete ev;
}

void DistSetCommReq::process( Event* _ev ) {
	m_req->setValue( this, static_cast<CommRespEvent*>(_ev) );
}

void DistComm::startLog( PWR_AttrName attr, CommReq* req )
{
	DBGX("%s\n",attrNameToString(attr));
	DistStartLogCommReq* distReq = static_cast<DistStartLogCommReq*>(req);

	CommLogReqEvent* ev = new CommLogReqEvent;	
	ev->commID = m_commID;
	ev->op = CommEvent::Start;
	ev->id = (EventId) req;	
	ev->attrName = attr; 
	m_ec->sendEvent( ev );
	delete ev;
}

void DistStartLogCommReq::process( Event* _ev ) {
	DBGX("\n");
	m_req->setStatus( this, static_cast<CommRespEvent*>(_ev) );
}

void DistStopLogCommReq::process( Event* _ev ) {
	DBGX("\n");
	m_req->setStatus( this, static_cast<CommRespEvent*>(_ev) );
}

void DistGetSamplesCommReq::process( Event* _ev ) {
	DBGX("\n");
	m_req->getSamples( this, static_cast<CommGetSamplesRespEvent*>(_ev) );
}

void DistComm::stopLog( PWR_AttrName attr, CommReq* req )
{
	DBGX("%s\n",attrNameToString(attr));
	DistStopLogCommReq* distReq = static_cast<DistStopLogCommReq*>(req);

	CommLogReqEvent* ev = new CommLogReqEvent;	
	ev->commID = m_commID;
	ev->op = CommEvent::Stop;
	ev->id = (EventId) req;	
	ev->attrName = attr; 
	m_ec->sendEvent( ev );
	delete ev;
}

void DistComm::getSamples( PWR_AttrName attr, PWR_Time* time,
				double period, unsigned int count, CommReq* req )
{
	DBGX("%s period=%f count=%d\n",attrNameToString(attr), period, count );
	DistGetSamplesCommReq* distReq = static_cast<DistGetSamplesCommReq*>(req);

	CommGetSamplesReqEvent* ev = new CommGetSamplesReqEvent;	
	ev->commID = m_commID;
	ev->id = (EventId) req;	
	ev->attrName = attr; 
	ev->startTime = *time;
	ev->period = period;
	ev->count = count;
	m_ec->sendEvent( ev );
	delete ev;
}
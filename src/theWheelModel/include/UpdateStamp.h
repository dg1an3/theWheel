//////////////////////////////////////////////////////////////////////
// UpdateStamp.h: interface for the UpdateStamp class.
//
// Copyright (C) 2007
// $Id$
//////////////////////////////////////////////////////////////////////

#pragma once

#define BEGIN_NAMESPACE(label) namespace label {
#define END_NAMESPACE(label) } 

BEGIN_NAMESPACE(theWheel)

//////////////////////////////////////////////////////////////////////
// class UpdateStamp
// 
// contains logic for maintaining serial update stamps
//////////////////////////////////////////////////////////////////////
class UpdateStamp
{
public:

	// create new stamp, initialized to zero (so will always require update)
	UpdateStamp(void)
		: m_nUpdateStamp(0) { }

	// updates this stamp
	void Update()
	{
		m_nUpdateStamp = m_nNextUpdateStamp;
		m_nNextUpdateStamp++;
	}

	// determines if an update is needed
	bool CheckNeedsUpdate(const UpdateStamp& independent)
	{
		return (m_nUpdateStamp <= independent.m_nUpdateStamp);
	}

private:

	// stores this update stamp
	unsigned long m_nUpdateStamp;

	// holds the next update serial number
	static unsigned long m_nNextUpdateStamp;

};

END_NAMESPACE(theWheel)


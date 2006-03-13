//////////////////////////////////////////////////////////////////////
// Observer.h: interface for the CObservableEvent class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// forward declaration of the CObservableEvent class
//////////////////////////////////////////////////////////////////////
class CObservableEvent;

//////////////////////////////////////////////////////////////////////
// defines the ChangeFunction which is called when a change occurs
//////////////////////////////////////////////////////////////////////
typedef void (CObject::*ListenerFunction)(CObservableEvent *, void *);

//////////////////////////////////////////////////////////////////////
// class CObservableEvent
// 
// a CObservableObject fires change events that can be processed by 
// an observer
//////////////////////////////////////////////////////////////////////
class CObservableEvent : public CObject
{
public:
	// creates an event for the parent object
	CObservableEvent(CObject *pParent = NULL);

	// includes dynamic type information
	DECLARE_DYNAMIC(CObservableEvent)

	// returns the parent of this event
	CObject *GetParent();

	// accessors for the observer list
	void AddObserver(CObject *pObserver, ListenerFunction func) const;
	void RemoveObserver(CObject *pObserver, ListenerFunction func) const;

	// called to fire a change
	void Fire(void *pValue = NULL);

private:
	// the parent object of this event
	CObject *m_pParent;

	// the array of observers
	mutable CArray<CObject *, CObject *> m_arrObservers;

	// the array of member functions
	mutable CArray<ListenerFunction, ListenerFunction> m_arrFunctions;
};

#endif // !defined(AFX_OBSERVER_H__AAA9A381_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)

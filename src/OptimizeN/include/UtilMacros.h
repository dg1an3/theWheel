//////////////////////////////////////////////////////////////////////
// UtilMacros.h: standard utility macros
//
// Copyright (C) 1999-2001
// $Id: UtilMacros.h,v 1.1 2007/10/29 01:43:52 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// Macros for attributes
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE_VAL(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	TYPE Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(TYPE value)		\
	{ m_##NAME = value; }

//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; }


//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE_EVT(NAME, TYPE, EVT) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; EVT.Fire(this); }


//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE_GI(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value);


//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE_PTR(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue)			\
	{ m_p##NAME = pValue; }


//////////////////////////////////////////////////////////////////////
#define DECLARE_ATTRIBUTE_PTR_GI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);

//////////////////////////////////////////////////////////////////////
// Macros for serialization
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
#define SERIALIZE_VALUE(ar, value) \
	if (ar.IsLoading())				\
	{								\
		ar >> value;				\
	}								\
	else							\
	{								\
		ar << value;				\
	}

//////////////////////////////////////////////////////////////////////
#define SERIALIZE_ARRAY(ar, array) \
	if (ar.IsLoading())				\
	{								\
		array.RemoveAll();			\
	}								\
	array.Serialize(ar);



//////////////////////////////////////////////////////////////////////
#define CHECK_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	if (FAILED(hr))			\
	{						\
		return hr;			\
	}						\
}


//////////////////////////////////////////////////////////////////////
#define ASSERT_HRESULT(call) \
{														\
	HRESULT hr = call;				\
	ATLASSERT(SUCCEEDED(hr));	\
}


//////////////////////////////////////////////////////////////////////
// Macros for logging
//////////////////////////////////////////////////////////////////////

#define BEGIN_LOG_SECTION(name) { 
#define BEGIN_LOG_SECTION_(name) {
#define BEGIN_LOG_ONLY(name) if (0) {
#define EXIT_LOG_SECTION() 
#define END_LOG_SECTION() (void) 0; }
#define LOG 1 ? (void) 0 : ::AfxTrace
#define LOG_EXPR(expr)
#define LOG_EXPR_DESC(expr, desc)
#define LOG_EXPR_EXT(expr)
#define LOG_EXPR_EXT_DESC(expr, desc) 


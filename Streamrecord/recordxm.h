#include "stdafx.h"
#include "loadpref.h"
#include "StreamInstance.h"
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>

#ifndef _RECORDXM_H
#define _RECORDXM_H


class RecordXM : public StreamInstance
{
	public:
		RecordXM(STREAMRECORD_PREFERENCES *ppref=NULL);
		~RecordXM();


};

#endif

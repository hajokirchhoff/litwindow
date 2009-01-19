/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: sample_data.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#ifndef _SAMPLE_DATA_
#define _SAMPLE_DATA_
#pragma once

struct MyFrameData
{
    int m_radioBox;             ///< contains the radio box choice.
    litwindow::tstring m_strRadioBox;  ///< contains the radio box choice as string
};

//DECLARE_ADAPTER_TYPE(MyFrameData, _declspec())
#endif

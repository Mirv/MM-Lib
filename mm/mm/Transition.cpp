/******************************************************************************
 * Copyright (c) 2013-2014, Amsterdam University of Applied Sciences (HvA) and
 *                          Centrum Wiskunde & Informatica (CWI)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   * Riemer van Rozen - rozen@cwi.nl - HvA / CWI
 ******************************************************************************/
//
//  Transition.cpp
//  mm
//
//  Created by Riemer van Rozen on 7/10/13.
//

#include <stdio.h>
#include <stdlib.h>
#include "YYLTYPE.h"
#include "Types.h"
#include "Recyclable.h"
#include "Vector.h"
#include "Recycler.h"
#include "Location.h"
#include "String.h"
#include "Name.h"
#include "Element.h"
#include "Transformation.h"
#include "Transition.h"

const MM::CHAR * MM::Transition::STEP_STR = "step";
const MM::UINT32 MM::Transition::STEP_LEN =
strlen(MM::Transition::STEP_STR);

MM::Transition::Transition(MM::Vector<MM::Element *> * elements):
  MM::Transformation(elements)
{
  this->loc = MM_NULL;
  this->str = MM_NULL;
}

MM::Transition::Transition(MM::Vector<MM::Element *> * elements,
                           MM::Location * loc):
  MM::Transformation(elements)
{
  this->loc = loc;
  this->str = MM_NULL;
}

MM::Transition::~Transition()
{
  this->loc = MM_NULL;
  this->str = MM_NULL;
}

MM::VOID MM::Transition::recycle(MM::Recycler * r)
{
  if(loc != MM_NULL)
  {
    loc->recycle(r);
  }
  if(str != MM_NULL)
  {
    str->recycle(r);
  }
  MM::Transformation::recycle(r);
}

MM::TID MM::Transition::getTypeId()
{
  return MM::T_Transition;
}

MM::BOOLEAN MM::Transition::instanceof(MM::TID tid)
{
  if(tid == MM::T_Transition)
  {
    return MM_TRUE;
  }
  else
  {
    return MM::Transformation::instanceof(tid);
  }
}

MM::VOID MM::Transition::toString(MM::String * buf)
{
  MM::Transformation::toString(buf);
  buf->append((MM::CHAR*)MM::Transition::STEP_STR,
              MM::Transition::STEP_LEN);
  buf->linebreak();
}
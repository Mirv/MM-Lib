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
/*!
 * \namespace MM
 * \class     SourceNodeBehavior
 * \brief     The SourceNodeBehavior abstraction defines the behavior
 *            of source nodes.
 * \note      Strategy
 * \file      SourceNodeBehavior.h
 * \author    Riemer van Rozen
 * \date      July 11th 2013
 */
/******************************************************************************/

#ifndef __mm__SourceNodeBehavior__
#define __mm__SourceNodeBehavior__
namespace MM
{
  class SourceNodeBehavior : public MM::NodeBehavior
  {
  private:
    static const MM::CHAR * SOURCE_STR;
    static const MM::UINT32 SOURCE_LEN;
  public:
    SourceNodeBehavior(MM::NodeBehavior::IO io, MM::NodeBehavior::When when);
    ~SourceNodeBehavior();
    MM::VOID recycle(MM::Recycler * r);
    MM::TID getTypeId();
    MM::BOOLEAN instanceof(MM::TID tid);
    MM::UINT32 getCreateMessage();
    MM::UINT32 getUpdateMessage();
    MM::UINT32 getDeleteMessage();
    
    //push any has inherited behavior
    
    MM::VOID stepPullAny(MM::Node * node,
                         MM::Instance * i,
                         MM::Vector<MM::NodeWorkItem *> * work,
                         MM::Machine * m,
                         MM::Transition * tr);
    
    MM::VOID stepPullAll(MM::Node * node,
                         MM::Instance * i,
                         MM::Vector<MM::NodeWorkItem *> * work,
                         MM::Machine * m,
                         MM::Transition * tr);
    
    MM::VOID stepPushAll(MM::Node * node,
                         MM::Instance * i,
                         MM::Vector<MM::NodeWorkItem *> * work,
                         MM::Machine * m,
                         MM::Transition * tr);
    
    //instance manipulation during transitions
    MM::VOID begin(MM::Instance * i, MM::Machine * m, MM::Node * n);
    MM::VOID end(MM::Instance * i, MM::Machine * m, MM::Node * n);
    MM::VOID change(MM::Instance * i, MM::Machine * m, MM::Node * n);
    //instance manipulation during transitions
    MM::VOID add(MM::Instance * i,
                 MM::Machine  * m,
                 MM::Node     * n,
                 MM::UINT32     amount);
    MM::VOID sub(MM::Instance * i,
                 MM::Machine  * m,
                 MM::Node     * n,
                 MM::UINT32     amount);
    MM::INT32 getCapacity(MM::Instance * i, MM::Node * n, MM::Machine * m);
    MM::INT32 getResources(MM::Instance * i, MM::Node * n, MM::Machine * m);
    MM::BOOLEAN hasCapacity(MM::Instance * i, MM::Node * n, MM::UINT32 amount, MM::Machine * m);
    MM::BOOLEAN hasResources(MM::Instance * i, MM::Node * n, MM::UINT32 amount, MM::Machine * m);
    
    MM::VOID toString(MM::String * buf);
    MM::VOID toString(MM::String * buf, MM::Name * name);
  };
}
#endif /* defined(__mm__SourceNodeBehavior__) */
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
 * \class     InterfaceNode
 * \brief     The InterfaceNode abstraction defines interface nodes on
 *            declarations and instance pools that result from
 *            observable nodes inside their respective definitions.
 * \file      InterfaceNode.h
 * \author    Riemer van Rozen
 * \date      October 8th 2013
 */
/******************************************************************************/

#ifndef __mm__InterfaceNode__
#define __mm__InterfaceNode__

namespace MM
{
  class InterfaceNode : public MM::Node
  {
  private:
    MM::Element * decl;
    MM::Node    * ref;
    MM::Edge    * alias;
  public:
    InterfaceNode(MM::Name    * name,
                  MM::Element * decl,
                  MM::Node    * ref);
    ~InterfaceNode();
    MM::VOID recycle(MM::Recycler *r);
    MM::TID getTypeId();
    MM::BOOLEAN instanceof(MM::TID tid);
    
    MM::Edge * getAlias();
    MM::VOID setAlias(MM::Edge * edge);

    MM::NodeBehavior * getBehavior();
    MM::Element * getDeclaration();
    MM::Node * getNode();    

    MM::VOID add(MM::Instance * i,
                 MM::UINT32 amount);
    MM::VOID sub(MM::Instance * i,
                 MM::UINT32 amount);
    MM::INT32 getCapacity(MM::Instance * i, MM::Machine * m);
    MM::INT32 getResources(MM::Instance * i, MM::Machine * m);
    MM::BOOLEAN hasCapacity(MM::Instance * i,
                            MM::UINT32 amount, MM::Machine * m);
    MM::BOOLEAN hasResources(MM::Instance * i,
                             MM::UINT32 amount, MM::Machine * m);
    
    MM::VOID activateTriggerTargets(MM::Instance * i, MM::Machine * m);
    MM::VOID toString(MM::String * buf);
  };
}
#endif /* defined(__mm__InterfaceNode__) */
